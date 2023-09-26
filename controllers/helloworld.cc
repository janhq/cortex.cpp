#include "helloworld.h"
#include "llama.h"


void helloworld::asyncHandleHttpRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

    auto lock = llama.lock();

    llama.rewind();

    llama_reset_timings(llama.ctx);

    parse_options_completion(json::parse(req.body), llama);

    if (!llama.loadGrammar()) {
      res.status = 400;
      return;
    }

    llama.loadPrompt();
    llama.beginCompletion();

    size_t stop_pos = std::string::npos;

    while (llama.has_next_token) {
      const completion_token_output token_with_probs = llama.doCompletion();
      const std::string token_text =
          token_with_probs.tok == -1
              ? ""
              : llama_token_to_piece(llama.ctx, token_with_probs.tok);

      stop_pos = llama.findStoppingStrings(llama.generated_text,
                                           token_text.size(), STOP_FULL);
    }

    if (stop_pos == std::string::npos) {
      stop_pos =
          llama.findStoppingStrings(llama.generated_text, 0, STOP_PARTIAL);
    }
    if (stop_pos != std::string::npos) {
      llama.generated_text.erase(llama.generated_text.begin() + stop_pos,
                                 llama.generated_text.end());
    }

    auto probs = llama.generated_token_probs;
    if (llama.params.n_probs > 0 && llama.stopped_word) {
      const std::vector<llama_token> stop_word_toks =
          llama_tokenize(llama.ctx, llama.stopping_word, false);
      probs = std::vector<completion_token_output>(
          llama.generated_token_probs.begin(),
          llama.generated_token_probs.end() - stop_word_toks.size());
    }

    const json data = format_final_response(llama, llama.generated_text, probs);

    llama_print_timings(llama.ctx);


  callback(resp);
}
