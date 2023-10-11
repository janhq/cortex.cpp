#include <drogon/HttpTypes.h>
#if defined(_WIN32)
#define NOMINMAX
#endif

#pragma once

#include "build-info.h"
#include "common.h"
#include "controllers/nitro_utils.h"
#include "grammar-parser.h"
#include "llama.h"
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpController.h>
#include <drogon/HttpSimpleController.h>
#include <exception>
#include <regex>
#include <trantor/utils/Logger.h>

#ifndef NDEBUG
// crash the server in debug mode, otherwise send an http 500 error
#define CPPHTTPLIB_NO_EXCEPTIONS 1
#endif

#include "json.hpp"

// auto generated files (update with ./deps.sh)

#include <cstddef>

#ifndef SERVER_VERBOSE
#define SERVER_VERBOSE 1
#endif

using json = nlohmann::json;

struct server_params {
  std::string hostname = "127.0.0.1";
  std::string public_path = "examples/server/public";
  int32_t port = 8080;
  int32_t read_timeout = 600;
  int32_t write_timeout = 600;
};

// completion token output with probabilities
struct completion_token_output {
  struct token_prob {
    llama_token tok;
    float prob;
  };

  std::vector<token_prob> probs;
  llama_token tok;
};

static size_t common_part(const std::vector<llama_token> &a,
                          const std::vector<llama_token> &b) {
  size_t i;
  for (i = 0; i < a.size() && i < b.size() && a[i] == b[i]; i++) {
  }
  return i;
}

enum stop_type {
  STOP_FULL,
  STOP_PARTIAL,
};

static bool ends_with(const std::string &str, const std::string &suffix) {
  return str.size() >= suffix.size() &&
         0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static size_t find_partial_stop_string(const std::string &stop,
                                       const std::string &text) {
  if (!text.empty() && !stop.empty()) {
    const char text_last_char = text.back();
    for (int64_t char_index = stop.size() - 1; char_index >= 0; char_index--) {
      if (stop[char_index] == text_last_char) {
        const std::string current_partial = stop.substr(0, char_index + 1);
        if (ends_with(text, current_partial)) {
          return text.size() - char_index - 1;
        }
      }
    }
  }
  return std::string::npos;
}

template <class Iter>
static std::string tokens_to_str(llama_context *ctx, Iter begin, Iter end) {
  std::string ret;
  for (; begin != end; ++begin) {
    ret += llama_token_to_piece(ctx, *begin);
  }
  return ret;
}

static void server_log(const char *level, const char *function, int line,
                       const char *message,
                       const nlohmann::ordered_json &extra) {
  nlohmann::ordered_json log{
      {"timestamp", time(nullptr)}, {"level", level},
      {"function", function},       {"line", line},
      {"message", message},
  };

  if (!extra.empty()) {
    log.merge_patch(extra);
  }

  const std::string str =
      log.dump(-1, ' ', false, json::error_handler_t::replace);
  LOG_INFO << str;
}

// format incomplete utf-8 multibyte character for output
static std::string tokens_to_output_formatted_string(const llama_context *ctx,
                                                     const llama_token token) {
  std::string out = token == -1 ? "" : llama_token_to_piece(ctx, token);
  // if the size is 1 and first bit is 1, meaning it's a partial character
  //   (size > 1 meaning it's already a known token)
  if (out.size() == 1 && (out[0] & 0x80) == 0x80) {
    std::stringstream ss;
    ss << std::hex << (out[0] & 0xff);
    std::string res(ss.str());
    out = "byte: \\x" + res;
  }
  return out;
}

// convert a vector of completion_token_output to json
static json
probs_vector_to_json(const llama_context *ctx,
                     const std::vector<completion_token_output> &probs) {
  json out = json::array();
  for (const auto &prob : probs) {
    json probs_for_token = json::array();
    for (const auto &p : prob.probs) {
      std::string tok_str = tokens_to_output_formatted_string(ctx, p.tok);
      probs_for_token.push_back(json{
          {"tok_str", tok_str},
          {"prob", p.prob},
      });
    }
    std::string tok_str = tokens_to_output_formatted_string(ctx, prob.tok);
    out.push_back(json{
        {"content", tok_str},
        {"probs", probs_for_token},
    });
  }
  return out;
}

static bool server_verbose = false;

#if SERVER_VERBOSE != 1
#define LOG_VERBOSE(MSG, ...)
#else
#define LOG_VERBOSE(MSG, ...)                                                  \
  do {                                                                         \
    if (server_verbose) {                                                      \
      server_log("VERBOSE", __func__, __LINE__, MSG, __VA_ARGS__);             \
    }                                                                          \
  } while (0)
#endif

#define LOG_ERROR_LLAMA(MSG, ...)                                              \
  server_log("ERROR", __func__, __LINE__, MSG, __VA_ARGS__)
#define LOG_WARNING_LLAMA(MSG, ...)                                            \
  server_log("WARNING", __func__, __LINE__, MSG, __VA_ARGS__)
#define LOG_INFO_LLAMA(MSG, ...)                                               \
  server_log("INFO", __func__, __LINE__, MSG, __VA_ARGS__)

struct llama_server_context {
  bool stream = false;
  bool has_next_token = false;
  std::string generated_text;
  std::vector<completion_token_output> generated_token_probs;

  size_t num_prompt_tokens = 0;
  size_t num_tokens_predicted = 0;
  size_t n_past = 0;
  size_t n_remain = 0;

  json prompt;
  std::vector<llama_token> embd;
  std::vector<llama_token> last_n_tokens;

  llama_model *model = nullptr;
  llama_context *ctx = nullptr;
  gpt_params params;
  int n_ctx;

  grammar_parser::parse_state parsed_grammar;
  llama_grammar *grammar = nullptr;

  bool truncated = false;
  bool stopped_eos = false;
  bool stopped_word = false;
  bool stopped_limit = false;
  std::string stopping_word;
  int32_t multibyte_pending = 0;

  std::mutex mutex;

  std::unique_lock<std::mutex> lock() {
    return std::unique_lock<std::mutex>(mutex);
  }

  ~llama_server_context() {
    if (ctx) {
      llama_free(ctx);
      ctx = nullptr;
    }
    if (model) {
      llama_free_model(model);
      model = nullptr;
    }
  }

  void rewind() {
    params.antiprompt.clear();
    params.grammar.clear();
    num_prompt_tokens = 0;
    num_tokens_predicted = 0;
    generated_text = "";
    generated_text.reserve(n_ctx);
    generated_token_probs.clear();
    truncated = false;
    stopped_eos = false;
    stopped_word = false;
    stopped_limit = false;
    stopping_word = "";
    multibyte_pending = 0;
    n_remain = 0;
    n_past = 0;

    if (grammar != nullptr) {
      llama_grammar_free(grammar);
      grammar = nullptr;
    }
  }

  bool loadModel(const gpt_params &params_) {
    params = params_;
    std::tie(model, ctx) = llama_init_from_gpt_params(params);
    if (model == nullptr) {
      LOG_ERROR_LLAMA("unable to load model", {{"model", params_.model}});
      return false;
    }
    n_ctx = llama_n_ctx(ctx);
    last_n_tokens.resize(n_ctx);
    std::fill(last_n_tokens.begin(), last_n_tokens.end(), 0);
    return true;
  }

  std::vector<llama_token> tokenize(const json &json_prompt,
                                    bool add_bos) const {
    // If `add_bos` is true, we only add BOS, when json_prompt is a string,
    // or the first element of the json_prompt array is a string.
    std::vector<llama_token> prompt_tokens;

    if (json_prompt.is_array()) {
      bool first = true;
      for (const auto &p : json_prompt) {
        if (p.is_string()) {
          auto s = p.template get<std::string>();
          std::vector<llama_token> p;
          if (first) {
            p = ::llama_tokenize(ctx, s, add_bos);
            first = false;
          } else {
            p = ::llama_tokenize(ctx, s, false);
          }
          prompt_tokens.insert(prompt_tokens.end(), p.begin(), p.end());
        } else {
          if (first) {
            first = false;
          }
          prompt_tokens.push_back(p.template get<llama_token>());
        }
      }
    } else {
      auto s = json_prompt.template get<std::string>();
      prompt_tokens = ::llama_tokenize(ctx, s, add_bos);
    }

    return prompt_tokens;
  }

  bool loadGrammar() {
    if (!params.grammar.empty()) {
      parsed_grammar = grammar_parser::parse(params.grammar.c_str());
      // will be empty (default) if there are parse errors
      if (parsed_grammar.rules.empty()) {
        LOG_ERROR_LLAMA("grammar parse error", {{"grammar", params.grammar}});
        return false;
      }
      grammar_parser::print_grammar(stderr, parsed_grammar);

      {
        auto it = params.logit_bias.find(llama_token_eos(ctx));
        if (it != params.logit_bias.end() && it->second == -INFINITY) {
          LOG_WARNING_LLAMA(
              "EOS token is disabled, which will cause most grammars to fail",
              {});
        }
      }

      std::vector<const llama_grammar_element *> grammar_rules(
          parsed_grammar.c_rules());
      grammar = llama_grammar_init(grammar_rules.data(), grammar_rules.size(),
                                   parsed_grammar.symbol_ids.at("root"));
    }
    return true;
  }

  void loadInfill() {
    auto prefix_tokens = tokenize(params.input_prefix, true); // always add BOS
    auto suffix_tokens = tokenize(params.input_suffix, true); // always add BOS
    prefix_tokens.insert(prefix_tokens.begin(), llama_token_prefix(ctx));
    prefix_tokens.insert(prefix_tokens.end(), llama_token_suffix(ctx));
    prefix_tokens.insert(prefix_tokens.end(), suffix_tokens.begin(),
                         suffix_tokens.end());
    prefix_tokens.push_back(llama_token_middle(ctx));
    auto prompt_tokens = prefix_tokens;

    num_prompt_tokens = prompt_tokens.size();

    if (params.n_keep < 0) {
      params.n_keep = (int)num_prompt_tokens;
    }
    params.n_keep = std::min(params.n_ctx - 4, params.n_keep);

    // if input prompt is too big, truncate like normal
    if (num_prompt_tokens >= (size_t)params.n_ctx) {
      printf("Input prompt is too big, truncating. Can only take %d tokens but "
             "got %zu\n",
             params.n_ctx, num_prompt_tokens);
      // todo we probably want to cut from both sides
      const int n_left = (params.n_ctx - params.n_keep) / 2;
      std::vector<llama_token> new_tokens(
          prompt_tokens.begin(), prompt_tokens.begin() + params.n_keep);
      const int erased_blocks =
          (num_prompt_tokens - params.n_keep - n_left - 1) / n_left;
      new_tokens.insert(new_tokens.end(),
                        prompt_tokens.begin() + params.n_keep +
                            erased_blocks * n_left,
                        prompt_tokens.end());
      std::copy(prompt_tokens.end() - params.n_ctx, prompt_tokens.end(),
                last_n_tokens.begin());

      LOG_VERBOSE("input truncated",
                  {
                      {"n_ctx", params.n_ctx},
                      {"n_keep", params.n_keep},
                      {"n_left", n_left},
                      {"new_tokens", tokens_to_str(ctx, new_tokens.cbegin(),
                                                   new_tokens.cend())},
                  });

      truncated = true;
      prompt_tokens = new_tokens;
    } else {
      const size_t ps = num_prompt_tokens;
      std::fill(last_n_tokens.begin(), last_n_tokens.end() - ps, 0);
      std::copy(prompt_tokens.begin(), prompt_tokens.end(),
                last_n_tokens.end() - ps);
    }

    // compare the evaluated prompt with the new prompt
    n_past = common_part(embd, prompt_tokens);
    embd = prompt_tokens;
    if (n_past == num_prompt_tokens) {
      // we have to evaluate at least 1 token to generate logits.
      printf("we have to evaluate at least 1 token to generate logits\n");
      n_past--;
    }

    LOG_VERBOSE("prompt ingested",
                {
                    {"n_past", n_past},
                    {"cached",
                     tokens_to_str(ctx, embd.cbegin(), embd.cbegin() + n_past)},
                    {"to_eval",
                     tokens_to_str(ctx, embd.cbegin() + n_past, embd.cend())},
                });

    has_next_token = true;
  }
  void loadPrompt() {
    auto prompt_tokens = tokenize(prompt, true); // always add BOS

    num_prompt_tokens = prompt_tokens.size();

    if (params.n_keep < 0) {
      params.n_keep = (int)num_prompt_tokens;
    }
    params.n_keep = std::min(n_ctx - 4, params.n_keep);

    // if input prompt is too big, truncate like normal
    if (num_prompt_tokens >= (size_t)n_ctx) {
      const int n_left = (n_ctx - params.n_keep) / 2;
      std::vector<llama_token> new_tokens(
          prompt_tokens.begin(), prompt_tokens.begin() + params.n_keep);
      const int erased_blocks =
          (num_prompt_tokens - params.n_keep - n_left - 1) / n_left;
      new_tokens.insert(new_tokens.end(),
                        prompt_tokens.begin() + params.n_keep +
                            erased_blocks * n_left,
                        prompt_tokens.end());
      std::copy(prompt_tokens.end() - n_ctx, prompt_tokens.end(),
                last_n_tokens.begin());

      LOG_VERBOSE("input truncated",
                  {
                      {"n_ctx", n_ctx},
                      {"n_keep", params.n_keep},
                      {"n_left", n_left},
                      {"new_tokens", tokens_to_str(ctx, new_tokens.cbegin(),
                                                   new_tokens.cend())},
                  });

      truncated = true;
      prompt_tokens = new_tokens;
    } else {
      const size_t ps = num_prompt_tokens;
      std::fill(last_n_tokens.begin(), last_n_tokens.end() - ps, 0);
      std::copy(prompt_tokens.begin(), prompt_tokens.end(),
                last_n_tokens.end() - ps);
    }

    // compare the evaluated prompt with the new prompt
    n_past = common_part(embd, prompt_tokens);

    // since #3228 we now have to manually manage the KV cache
    llama_kv_cache_seq_rm(ctx, 0, n_past, params.n_ctx);

    embd = prompt_tokens;
    if (n_past == num_prompt_tokens) {
      // we have to evaluate at least 1 token to generate logits.
      n_past--;
    }

    LOG_VERBOSE("prompt ingested",
                {
                    {"n_past", n_past},
                    {"cached",
                     tokens_to_str(ctx, embd.cbegin(), embd.cbegin() + n_past)},
                    {"to_eval",
                     tokens_to_str(ctx, embd.cbegin() + n_past, embd.cend())},
                });

    has_next_token = true;
  }

  void beginCompletion() {
    // number of tokens to keep when resetting context
    n_remain = params.n_predict;
    llama_set_rng_seed(ctx, params.seed);
  }

  completion_token_output nextToken() {
    completion_token_output result;
    result.tok = -1;

    if (embd.size() >= (size_t)n_ctx) {
      // Shift context

      const int n_left = n_past - params.n_keep - 1;
      const int n_discard = n_left / 2;

      llama_kv_cache_seq_rm(ctx, 0, params.n_keep + 1,
                            params.n_keep + n_discard + 1);
      llama_kv_cache_seq_shift(ctx, 0, params.n_keep + 1 + n_discard, n_past,
                               -n_discard);

      for (size_t i = params.n_keep + 1 + n_discard; i < embd.size(); i++) {
        embd[i - n_discard] = embd[i];
      }
      embd.resize(embd.size() - n_discard);

      n_past -= n_discard;

      truncated = true;
      LOG_VERBOSE("input truncated", {
                                         {"n_ctx", n_ctx},
                                         {"n_keep", params.n_keep},
                                         {"n_left", n_left},
                                     });
    }

    while (n_past < embd.size()) {
      int n_eval = (int)embd.size() - n_past;
      if (n_eval > params.n_batch) {
        n_eval = params.n_batch;
      }

      if (llama_decode(ctx,
                       llama_batch_get_one(&embd[n_past], n_eval, n_past, 0))) {
        LOG_ERROR_LLAMA("failed to eval",
                        {
                            {"n_eval", n_eval},
                            {"n_past", n_past},
                            {"embd", tokens_to_str(ctx, embd.cbegin() + n_past,
                                                   embd.cend())},
                        });
        has_next_token = false;
        return result;
      }
      n_past += n_eval;
    }

    if (params.n_predict == 0) {
      has_next_token = false;
      result.tok = llama_token_eos(ctx);
      return result;
    }

    // out of user input, sample next token
    const float temp = params.temp;
    const int32_t top_k =
        params.top_k <= 0 ? llama_n_vocab(model) : params.top_k;
    const float top_p = params.top_p;
    const float tfs_z = params.tfs_z;
    const float typical_p = params.typical_p;
    const int32_t repeat_last_n =
        params.repeat_last_n < 0 ? n_ctx : params.repeat_last_n;
    const float repeat_penalty = params.repeat_penalty;
    const float alpha_presence = params.presence_penalty;
    const float alpha_frequency = params.frequency_penalty;
    const int mirostat = params.mirostat;
    const float mirostat_tau = params.mirostat_tau;
    const float mirostat_eta = params.mirostat_eta;
    const bool penalize_nl = params.penalize_nl;
    const int32_t n_probs = params.n_probs;

    {
      auto *logits = llama_get_logits(ctx);
      auto n_vocab = llama_n_vocab(model);

      // Apply params.logit_bias map
      for (const auto &it : params.logit_bias) {
        logits[it.first] += it.second;
      }

      std::vector<llama_token_data> candidates;
      candidates.reserve(n_vocab);
      for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
        candidates.emplace_back(
            llama_token_data{token_id, logits[token_id], 0.0f});
      }

      llama_token_data_array candidates_p = {candidates.data(),
                                             candidates.size(), false};

      // Apply penalties
      float nl_logit = logits[llama_token_nl(ctx)];
      auto last_n_repeat =
          std::min(std::min((int)last_n_tokens.size(), repeat_last_n), n_ctx);
      llama_sample_repetition_penalty(ctx, &candidates_p,
                                      last_n_tokens.data() +
                                          last_n_tokens.size() - last_n_repeat,
                                      last_n_repeat, repeat_penalty);
      llama_sample_frequency_and_presence_penalties(
          ctx, &candidates_p,
          last_n_tokens.data() + last_n_tokens.size() - last_n_repeat,
          last_n_repeat, alpha_frequency, alpha_presence);
      if (!penalize_nl) {
        logits[llama_token_nl(ctx)] = nl_logit;
      }

      if (grammar != nullptr) {
        llama_sample_grammar(ctx, &candidates_p, grammar);
      }

      if (temp <= 0) {
        // Greedy sampling
        result.tok = llama_sample_token_greedy(ctx, &candidates_p);
        if (n_probs > 0) {
          llama_sample_softmax(ctx, &candidates_p);
        }
      } else {
        if (mirostat == 1) {
          static float mirostat_mu = 2.0f * mirostat_tau;
          const int mirostat_m = 100;
          llama_sample_temp(ctx, &candidates_p, temp);
          result.tok = llama_sample_token_mirostat(ctx, &candidates_p,
                                                   mirostat_tau, mirostat_eta,
                                                   mirostat_m, &mirostat_mu);
        } else if (mirostat == 2) {
          static float mirostat_mu = 2.0f * mirostat_tau;
          llama_sample_temp(ctx, &candidates_p, temp);
          result.tok = llama_sample_token_mirostat_v2(
              ctx, &candidates_p, mirostat_tau, mirostat_eta, &mirostat_mu);
        } else {
          // Temperature sampling
          size_t min_keep = std::max(1, n_probs);
          llama_sample_top_k(ctx, &candidates_p, top_k, min_keep);
          llama_sample_tail_free(ctx, &candidates_p, tfs_z, min_keep);
          llama_sample_typical(ctx, &candidates_p, typical_p, min_keep);
          llama_sample_top_p(ctx, &candidates_p, top_p, min_keep);
          llama_sample_temp(ctx, &candidates_p, temp);
          result.tok = llama_sample_token(ctx, &candidates_p);
        }
      }

      if (grammar != nullptr) {
        llama_grammar_accept_token(ctx, grammar, result.tok);
      }

      for (size_t i = 0; i < std::min(candidates_p.size, (size_t)n_probs);
           ++i) {
        result.probs.push_back(
            {candidates_p.data[i].id, candidates_p.data[i].p});
      }

      last_n_tokens.erase(last_n_tokens.begin());
      last_n_tokens.push_back(result.tok);
      num_tokens_predicted++;
    }

    // add it to the context
    embd.push_back(result.tok);
    // decrement remaining sampling budget
    --n_remain;

    if (!embd.empty() && embd.back() == llama_token_eos(ctx)) {
      // stopping_word = llama_token_to_piece(ctx, embd.back());
      has_next_token = false;
      stopped_eos = true;
      LOG_VERBOSE("eos token found", {});
      return result;
    }

    has_next_token = params.n_predict == -1 || n_remain != 0;
    return result;
  }

  size_t findStoppingStrings(const std::string &text,
                             const size_t last_token_size,
                             const stop_type type) {
    size_t stop_pos = std::string::npos;
    for (const std::string &word : params.antiprompt) {
      size_t pos;
      if (type == STOP_FULL) {
        const size_t tmp = word.size() + last_token_size;
        const size_t from_pos = text.size() > tmp ? text.size() - tmp : 0;
        pos = text.find(word, from_pos);
      } else {
        pos = find_partial_stop_string(word, text);
      }
      if (pos != std::string::npos &&
          (stop_pos == std::string::npos || pos < stop_pos)) {
        if (type == STOP_FULL) {
          stopping_word = word;
          stopped_word = true;
          has_next_token = false;
        }
        stop_pos = pos;
      }
    }
    return stop_pos;
  }

  completion_token_output doCompletion() {
    auto token_with_probs = nextToken();

    const std::string token_text =
        token_with_probs.tok == -1
            ? ""
            : llama_token_to_piece(ctx, token_with_probs.tok);
    generated_text += token_text;

    if (params.n_probs > 0) {
      generated_token_probs.push_back(token_with_probs);
    }

    if (multibyte_pending > 0) {
      multibyte_pending -= token_text.size();
    } else if (token_text.size() == 1) {
      const char c = token_text[0];
      // 2-byte characters: 110xxxxx 10xxxxxx
      if ((c & 0xE0) == 0xC0) {
        multibyte_pending = 1;
        // 3-byte characters: 1110xxxx 10xxxxxx 10xxxxxx
      } else if ((c & 0xF0) == 0xE0) {
        multibyte_pending = 2;
        // 4-byte characters: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      } else if ((c & 0xF8) == 0xF0) {
        multibyte_pending = 3;
      } else {
        multibyte_pending = 0;
      }
    }

    if (multibyte_pending > 0 && !has_next_token) {
      has_next_token = true;
      n_remain++;
    }

    if (!has_next_token && n_remain == 0) {
      stopped_limit = true;
    }

    LOG_VERBOSE("next token",
                {
                    {"token", token_with_probs.tok},
                    {"token_text", tokens_to_output_formatted_string(
                                       ctx, token_with_probs.tok)},
                    {"has_next_token", has_next_token},
                    {"n_remain", n_remain},
                    {"num_tokens_predicted", num_tokens_predicted},
                    {"stopped_eos", stopped_eos},
                    {"stopped_word", stopped_word},
                    {"stopped_limit", stopped_limit},
                    {"stopping_word", stopping_word},
                });

    return token_with_probs;
  }

  std::vector<float> getEmbedding() {
    static const int n_embd = llama_n_embd(model);
    if (!params.embedding) {
      LOG_WARNING_LLAMA("embedding disabled",
                        {
                            {"params.embedding", params.embedding},
                        });
      return std::vector<float>(n_embd, 0.0f);
    }
    const float *data = llama_get_embeddings(ctx);
    std::vector<float> embedding(data, data + n_embd);
    return embedding;
  }
};

static void server_print_usage(const char *argv0, const gpt_params &params,
                               const server_params &sparams) {
  printf("usage: %s [options]\n", argv0);
  printf("\n");
  printf("options:\n");
  printf("  -h, --help            show this help message and exit\n");
  printf("  -v, --verbose         verbose output (default: %s)\n",
         server_verbose ? "enabled" : "disabled");
  printf("  -t N, --threads N     number of threads to use during computation "
         "(default: %d)\n",
         params.n_threads);
  printf("  -c N, --ctx-size N    size of the prompt context (default: %d)\n",
         params.n_ctx);
  printf("  --rope-freq-base N    RoPE base frequency (default: loaded from "
         "model)\n");
  printf("  --rope-freq-scale N   RoPE frequency scaling factor (default: "
         "loaded from model)\n");
  printf("  -b N, --batch-size N  batch size for prompt processing (default: "
         "%d)\n",
         params.n_batch);
  printf("  --memory-f32          use f32 instead of f16 for memory key+value "
         "(default: disabled)\n");
  printf("                        not recommended: doubles context memory "
         "required and no measurable increase in quality\n");
  if (llama_mlock_supported()) {
    printf("  --mlock               force system to keep model in RAM rather "
           "than swapping or compressing\n");
  }
  if (llama_mmap_supported()) {
    printf("  --no-mmap             do not memory-map model (slower load but "
           "may reduce pageouts if not using mlock)\n");
  }
  printf("  --numa                attempt optimizations that help on some NUMA "
         "systems\n");
#ifdef LLAMA_SUPPORTS_GPU_OFFLOAD
  printf("  -ngl N, --n-gpu-layers N\n");
  printf("                        number of layers to store in VRAM\n");
  printf("  -ts SPLIT --tensor-split SPLIT\n");
  printf("                        how to split tensors across multiple GPUs, "
         "comma-separated list of proportions, e.g. 3,1\n");
  printf(
      "  -mg i, --main-gpu i   the GPU to use for scratch and small tensors\n");
  printf("  -nommq, --no-mul-mat-q\n");
  printf("                        use cuBLAS instead of custom mul_mat_q CUDA "
         "kernels.\n");
  printf("                        Not recommended since this is both slower "
         "and uses more VRAM.\n");
#endif
  printf("  -m FNAME, --model FNAME\n");
  printf("                        model path (default: %s)\n",
         params.model.c_str());
  printf("  -a ALIAS, --alias ALIAS\n");
  printf("                        set an alias for the model, will be added as "
         "`model` field in completion response\n");
  printf("  --lora FNAME          apply LoRA adapter (implies --no-mmap)\n");
  printf("  --lora-base FNAME     optional model to use as a base for the "
         "layers modified by the LoRA adapter\n");
  printf(
      "  --host                ip address to listen (default  (default: %s)\n",
      sparams.hostname.c_str());
  printf("  --port PORT           port to listen (default  (default: %d)\n",
         sparams.port);
  printf("  --path PUBLIC_PATH    path from which to serve static files "
         "(default %s)\n",
         sparams.public_path.c_str());
  printf("  -to N, --timeout N    server read/write timeout in seconds "
         "(default: %d)\n",
         sparams.read_timeout);
  printf(
      "  --embedding           enable embedding vector output (default: %s)\n",
      params.embedding ? "enabled" : "disabled");
  printf("\n");
}

static void server_params_parse(int argc, char **argv, server_params &sparams,
                                gpt_params &params) {
  gpt_params default_params;
  server_params default_sparams;
  std::string arg;
  bool invalid_param = false;

  for (int i = 1; i < argc; i++) {
    arg = argv[i];
    if (arg == "--port") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      sparams.port = std::stoi(argv[i]);
    } else if (arg == "--host") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      sparams.hostname = argv[i];
    } else if (arg == "--path") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      sparams.public_path = argv[i];
    } else if (arg == "--timeout" || arg == "-to") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      sparams.read_timeout = std::stoi(argv[i]);
      sparams.write_timeout = std::stoi(argv[i]);
    } else if (arg == "-m" || arg == "--model") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.model = argv[i];
    } else if (arg == "-a" || arg == "--alias") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.model_alias = argv[i];
    } else if (arg == "-h" || arg == "--help") {
      server_print_usage(argv[0], default_params, default_sparams);
      exit(0);
    } else if (arg == "-c" || arg == "--ctx-size" || arg == "--ctx_size") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.n_ctx = std::stoi(argv[i]);
    } else if (arg == "--rope-freq-base") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.rope_freq_base = std::stof(argv[i]);
    } else if (arg == "--rope-freq-scale") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.rope_freq_scale = std::stof(argv[i]);
    } else if (arg == "--memory-f32" || arg == "--memory_f32") {
      params.memory_f16 = false;
    } else if (arg == "--threads" || arg == "-t") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.n_threads = std::stoi(argv[i]);
    } else if (arg == "-b" || arg == "--batch-size") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.n_batch = std::stoi(argv[i]);
      params.n_batch = std::min(512, params.n_batch);
    } else if (arg == "--gpu-layers" || arg == "-ngl" ||
               arg == "--n-gpu-layers") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
#ifdef LLAMA_SUPPORTS_GPU_OFFLOAD
      params.n_gpu_layers = std::stoi(argv[i]);
#else
      LOG_WARNING_LLAMA(
          "Not compiled with GPU offload support, --n-gpu-layers option will "
          "be ignored. "
          "See main README.md for information on enabling GPU BLAS support",
          {{"n_gpu_layers", params.n_gpu_layers}});
#endif
    } else if (arg == "--tensor-split" || arg == "-ts") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
#ifdef GGML_USE_CUBLAS
      std::string arg_next = argv[i];

      // split string by , and /
      const std::regex regex{R"([,/]+)"};
      std::sregex_token_iterator it{arg_next.begin(), arg_next.end(), regex,
                                    -1};
      std::vector<std::string> split_arg{it, {}};
      GGML_ASSERT(split_arg.size() <= LLAMA_MAX_DEVICES);

      for (size_t i_device = 0; i_device < LLAMA_MAX_DEVICES; ++i_device) {
        if (i_device < split_arg.size()) {
          params.tensor_split[i_device] = std::stof(split_arg[i_device]);
        } else {
          params.tensor_split[i_device] = 0.0f;
        }
      }
#else
      LOG_WARNING_LLAMA("llama.cpp was compiled without cuBLAS. It is not "
                        "possible to set a tensor split.\n",
                        {});
#endif // GGML_USE_CUBLAS
    } else if (arg == "--no-mul-mat-q" || arg == "-nommq") {
#ifdef GGML_USE_CUBLAS
      params.mul_mat_q = false;
#else
      LOG_WARNING_LLAMA("warning: llama.cpp was compiled without cuBLAS. "
                        "Disabling mul_mat_q kernels has no effect.\n",
                        {});
#endif // GGML_USE_CUBLAS
    } else if (arg == "--main-gpu" || arg == "-mg") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
#ifdef GGML_USE_CUBLAS
      params.main_gpu = std::stoi(argv[i]);
#else
      LOG_WARNING_LLAMA("llama.cpp was compiled without cuBLAS. It is not "
                        "possible to set a main GPU.",
                        {});
#endif
    } else if (arg == "--lora") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.lora_adapter.push_back({argv[i], 1.0f});
      params.use_mmap = false;
    } else if (arg == "--lora-scaled") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      const char *lora_adapter = argv[i];
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.lora_adapter.push_back({lora_adapter, std::stof(argv[i])});
      params.use_mmap = false;
    } else if (arg == "--lora-base") {
      if (++i >= argc) {
        invalid_param = true;
        break;
      }
      params.lora_base = argv[i];
    } else if (arg == "-v" || arg == "--verbose") {
#if SERVER_VERBOSE != 1
      LOG_WARNING_LLAMA("server.cpp is not built with verbose logging.", {});
#else
      server_verbose = true;
#endif
    } else if (arg == "--mlock") {
      params.use_mlock = true;
    } else if (arg == "--no-mmap") {
      params.use_mmap = false;
    } else if (arg == "--numa") {
      params.numa = true;
    } else if (arg == "--embedding") {
      params.embedding = true;
    } else {
      fprintf(stderr, "error: unknown argument: %s\n", arg.c_str());
      server_print_usage(argv[0], default_params, default_sparams);
      exit(1);
    }
  }

  if (invalid_param) {
    fprintf(stderr, "error: invalid parameter for argument: %s\n", arg.c_str());
    server_print_usage(argv[0], default_params, default_sparams);
    exit(1);
  }
}

static json format_generation_settings(llama_server_context &llama) {
  const auto eos_bias =
      llama.params.logit_bias.find(llama_token_eos(llama.ctx));
  const bool ignore_eos = eos_bias != llama.params.logit_bias.end() &&
                          eos_bias->second < 0.0f &&
                          std::isinf(eos_bias->second);

  return json{
      {"n_ctx", llama.n_ctx},
      {"model", llama.params.model_alias},
      {"seed", llama.params.seed},
      {"temp", llama.params.temp},
      {"top_k", llama.params.top_k},
      {"top_p", llama.params.top_p},
      {"tfs_z", llama.params.tfs_z},
      {"typical_p", llama.params.typical_p},
      {"repeat_last_n", llama.params.repeat_last_n},
      {"repeat_penalty", llama.params.repeat_penalty},
      {"presence_penalty", llama.params.presence_penalty},
      {"frequency_penalty", llama.params.frequency_penalty},
      {"mirostat", llama.params.mirostat},
      {"mirostat_tau", llama.params.mirostat_tau},
      {"mirostat_eta", llama.params.mirostat_eta},
      {"penalize_nl", llama.params.penalize_nl},
      {"stop", llama.params.antiprompt},
      {"n_predict", llama.params.n_predict},
      {"n_keep", llama.params.n_keep},
      {"ignore_eos", ignore_eos},
      {"stream", llama.stream},
      {"logit_bias", llama.params.logit_bias},
      {"n_probs", llama.params.n_probs},
      {"grammar", llama.params.grammar},
  };
}

static json format_embedding_response(llama_server_context &llama) {
  return json{
      {"embedding", llama.getEmbedding()},
  };
}

static json format_timings(llama_server_context &llama) {
  const auto timings = llama_get_timings(llama.ctx);

  assert(timings.n_eval == ptrdiff_t(llama.num_tokens_predicted));

  return json{
      {"prompt_n", timings.n_p_eval},
      {"prompt_ms", timings.t_p_eval_ms},
      {"prompt_per_token_ms", timings.t_p_eval_ms / timings.n_p_eval},
      {"prompt_per_second", 1e3 / timings.t_p_eval_ms * timings.n_p_eval},

      {"predicted_n", timings.n_eval},
      {"predicted_ms", timings.t_eval_ms},
      {"predicted_per_token_ms", timings.t_eval_ms / timings.n_eval},
      {"predicted_per_second", 1e3 / timings.t_eval_ms * timings.n_eval},
  };
}

static json
format_final_response(llama_server_context &llama, const std::string &content,
                      const std::vector<completion_token_output> &probs) {

  json res = json{
      {"content", content},
      {"stop", true},
      {"model", llama.params.model_alias},
      {"tokens_predicted", llama.num_tokens_predicted},
      {"tokens_evaluated", llama.num_prompt_tokens},
      {"generation_settings", format_generation_settings(llama)},
      {"prompt", llama.prompt},
      {"truncated", llama.truncated},
      {"stopped_eos", llama.stopped_eos},
      {"stopped_word", llama.stopped_word},
      {"stopped_limit", llama.stopped_limit},
      {"stopping_word", llama.stopping_word},
      {"tokens_cached", llama.n_past},
      {"timings", format_timings(llama)},
  };

  if (llama.params.n_probs > 0) {
    res["completion_probabilities"] = probs_vector_to_json(llama.ctx, probs);
  }

  return res;
}

static json
format_partial_response(llama_server_context &llama, const std::string &content,
                        const std::vector<completion_token_output> &probs) {
  json res = json{
      {"content", content},
      {"stop", false},
  };

  if (llama.params.n_probs > 0) {
    res["completion_probabilities"] = probs_vector_to_json(llama.ctx, probs);
  }

  return res;
}

static json format_tokenizer_response(const std::vector<llama_token> &tokens) {
  return json{{"tokens", tokens}};
}

static json format_detokenized_response(std::string content) {
  return json{{"content", content}};
}

template <typename T>
static T json_value(const json &body, const std::string &key,
                    const T &default_value) {
  // Fallback null to default value
  return body.contains(key) && !body.at(key).is_null()
             ? body.value(key, default_value)
             : default_value;
}

static void parse_options_completion(const json &body,
                                     llama_server_context &llama) {
  gpt_params default_params;

  llama.stream = json_value(body, "stream", false);
  llama.params.n_predict =
      json_value(body, "n_predict", default_params.n_predict);
  llama.params.top_k = json_value(body, "top_k", default_params.top_k);
  llama.params.top_p = json_value(body, "top_p", default_params.top_p);
  llama.params.tfs_z = json_value(body, "tfs_z", default_params.tfs_z);
  llama.params.typical_p =
      json_value(body, "typical_p", default_params.typical_p);
  llama.params.repeat_last_n =
      json_value(body, "repeat_last_n", default_params.repeat_last_n);
  llama.params.temp = json_value(body, "temperature", default_params.temp);
  llama.params.repeat_penalty =
      json_value(body, "repeat_penalty", default_params.repeat_penalty);
  llama.params.presence_penalty =
      json_value(body, "presence_penalty", default_params.presence_penalty);
  llama.params.frequency_penalty =
      json_value(body, "frequency_penalty", default_params.frequency_penalty);
  llama.params.mirostat = json_value(body, "mirostat", default_params.mirostat);
  llama.params.mirostat_tau =
      json_value(body, "mirostat_tau", default_params.mirostat_tau);
  llama.params.mirostat_eta =
      json_value(body, "mirostat_eta", default_params.mirostat_eta);
  llama.params.penalize_nl =
      json_value(body, "penalize_nl", default_params.penalize_nl);
  llama.params.n_keep = json_value(body, "n_keep", default_params.n_keep);
  llama.params.seed = json_value(body, "seed", default_params.seed);
  llama.params.grammar = json_value(body, "grammar", default_params.grammar);
  llama.params.n_probs = json_value(body, "n_probs", default_params.n_probs);

  if (body.count("prompt") != 0) {
    llama.prompt = body["prompt"];
  } else {
    llama.prompt = "";
  }

  llama.params.logit_bias.clear();
  if (json_value(body, "ignore_eos", false)) {
    llama.params.logit_bias[llama_token_eos(llama.ctx)] = -INFINITY;
  }

  const auto &logit_bias = body.find("logit_bias");
  if (logit_bias != body.end() && logit_bias->is_array()) {
    const int n_vocab = llama_n_vocab(llama.model);
    for (const auto &el : *logit_bias) {
      if (el.is_array() && el.size() == 2 && el[0].is_number_integer()) {
        llama_token tok = el[0].get<llama_token>();
        if (tok >= 0 && tok < n_vocab) {
          if (el[1].is_number()) {
            llama.params.logit_bias[tok] = el[1].get<float>();
          } else if (el[1].is_boolean() && !el[1].get<bool>()) {
            llama.params.logit_bias[tok] = -INFINITY;
          }
        }
      }
    }
  }

  llama.params.antiprompt.clear();
  ế mà mua chưa, e tính nhờ mua 2 hộp thể const auto &stop = body.find("stop");
  if (stop != body.end() && stop->is_array()) {
    for (const auto &word : *stop) {
      if (!word.empty()) {
        llama.params.antiprompt.push_back(word);
      }
    }
  }

  LOG_VERBOSE("completion parameters parsed",
              format_generation_settings(llama));
}

static void parse_options_infill(const json &body,
                                 llama_server_context &llama) {
  if (body.count("input_prefix") != 0) {
    llama.params.input_prefix = body["input_prefix"];
  } else {
    llama.params.input_prefix = "";
  }
  if (body.count("input_suffix") != 0) {
    llama.params.input_suffix = body["input_suffix"];
  } else {
    llama.params.input_suffix = "";
  }
  parse_options_completion(body, llama);
}

// static void log_server_request(const Request &req, const Response &res)
//{
//     LOG_INFO("request", {
//                             {"remote_addr", req.remote_addr},
//                             {"remote_port", req.remote_port},
//                             {"status", res.status},
//                             {"method", req.method},
//                             {"path", req.path},
//                             {"params", req.params},
//                         });
//
//     LOG_VERBOSE("request", {
//                                {"request", req.body},
//                                {"response", res.body},
//                            });
// }
//
static bool is_at_eob(llama_server_context &server_context,
                      const llama_token *tokens, const size_t n_tokens) {
  return n_tokens &&
         tokens[n_tokens - 1] == llama_token_eos(server_context.ctx);
}

// Function matching type llama_beam_search_callback_fn_t.
// Custom callback example is called each time the beams lengths increase:
//  * Show progress by printing ',' following by number of convergent beam
//  tokens if any.
//  * When all beams converge to a common prefix, they are made available in
//  beams_state.beams[0].
//    This is also called when the stop condition is met.
//    Collect tokens into std::vector<llama_token> response which is pointed to
//    by callback_data.
static void beam_search_callback(void *callback_data,
                                 llama_beams_state beams_state) {
  auto &llama = *static_cast<llama_server_context *>(callback_data);
  // Mark beams as EOS as needed.
  for (size_t i = 0; i < beams_state.n_beams; ++i) {
    llama_beam_view &beam_view = beams_state.beam_views[i];
    if (!beam_view.eob &&
        is_at_eob(llama, beam_view.tokens, beam_view.n_tokens)) {
      beam_view.eob = true;
    }
  }
  printf(","); // Show progress
  if (const size_t n = beams_state.common_prefix_length) {
    llama.generated_token_probs.resize(llama.generated_token_probs.size() + n);
    assert(0u < beams_state.n_beams);
    const llama_token *tokens = beams_state.beam_views[0].tokens;
    const auto map = [](llama_token tok) {
      return completion_token_output{{}, tok};
    };
    std::transform(tokens, tokens + n, llama.generated_token_probs.end() - n,
                   map);
    printf("%zu", n);
  }
  fflush(stdout);
#if 0 // DEBUG: print current beams for this iteration
    std::cout << "\n\nCurrent beams:\n";
    for (size_t i=0 ; i < beams_state.n_beams ; ++i) {
        std::cout << "beams["<<i<<"]: " << ostream_beam_view{state.ctx,beams_state.beam_views[i]} << std::endl;
    }
#endif
}

struct token_translator {
  llama_context *ctx;
  std::string operator()(llama_token tok) const {
    return llama_token_to_piece(ctx, tok);
  }
  std::string operator()(const completion_token_output &cto) const {
    return (*this)(cto.tok);
  }
};

static void append_to_generated_text_from_generated_token_probs(
    llama_server_context &llama) {
  auto &gtps = llama.generated_token_probs;
  auto translator = token_translator{llama.ctx};
  auto add_strlen = [=](size_t sum, const completion_token_output &cto) {
    return sum + translator(cto).size();
  };
  const size_t len =
      std::accumulate(gtps.begin(), gtps.end(), size_t(0), add_strlen);
  if (llama.generated_text.capacity() < llama.generated_text.size() + len) {
    llama.generated_text.reserve(llama.generated_text.size() + len);
  }
  for (const completion_token_output &cto : gtps) {
    llama.generated_text += translator(cto);
  }
}
using namespace drogon;

namespace inferences {
class llamaCPP : public drogon::HttpController<llamaCPP> {
public:
  llamaCPP() {
    //    gpt_params params;
    //    auto conf = drogon::app().getCustomConfig();
    //    params.model = conf["llama_model_path"].asString();
    //    params.n_gpu_layers = conf["ngl"].asInt();
    //    params.n_ctx = conf["ctx_len"].asInt();
    //    params.embedding = conf["embedding"].asBool();
    // #ifdef GGML_USE_CUBLAS
    //    LOG_INFO << "Setting up GGML CUBLAS PARAMS";
    //    params.mul_mat_q = false;
    // #endif // GGML_USE_CUBLAS
    //    if (params.model_alias == "unknown") {
    //      params.model_alias = params.model;
    //    }
    //
    //    llama_backend_init(params.numa);
    //
    //    LOG_INFO_LLAMA("build info",
    //                   {{"build", BUILD_NUMBER}, {"commit", BUILD_COMMIT}});
    //    LOG_INFO_LLAMA("system info",
    //                   {
    //                       {"n_threads", params.n_threads},
    //                       {"total_threads",
    //                       std::thread::hardware_concurrency()},
    //                       {"system_info", llama_print_system_info()},
    //                   });
    //
    //    // load the model
    //    if (!llama.loadModel(params)) {
    //      LOG_ERROR << "Error loading the model will exit the program";
    //      std::terminate();
    //    }
    //    deprecate this if find no usecase
  }
  METHOD_LIST_BEGIN
  // list path definitions here;
  METHOD_ADD(llamaCPP::chatCompletion, "chat_completion", Post);
  METHOD_ADD(llamaCPP::embedding, "embedding", Post);
  METHOD_ADD(llamaCPP::loadModel, "loadmodel", Post);
  // PATH_ADD("/llama/chat_completion", Post);
  METHOD_LIST_END
  void chatCompletion(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback);
  void embedding(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback);
  void loadModel(const HttpRequestPtr &req,
                 std::function<void(const HttpResponsePtr &)> &&callback);

private:
  llama_server_context llama;
  bool model_loaded = false;
  size_t sent_count = 0;
  size_t sent_token_probs_index = 0;
};
}; // namespace inferences
