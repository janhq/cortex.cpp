#include "helloworld.h"
#include "llama.h"
#include <cstring>
#include <drogon/HttpResponse.h>

void helloworld::asyncHandleHttpRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

  auto lock = llama.lock();

  llama.rewind();

  llama_reset_timings(llama.ctx);

  llama.prompt = "Hello there have a great day";
  if (!llama.loadGrammar()) {
    LOG_ERROR << "Serious error happen here kekeke";
    return;
  }

  llama.loadPrompt();
  llama.beginCompletion();

  const auto chunked_content_provider =
      [&](char *pBuffer, std::size_t nBuffSize) -> std::size_t {
    while (llama.has_next_token) {
      const completion_token_output token_with_probs = llama.doCompletion();
      if (token_with_probs.tok == -1 || llama.multibyte_pending > 0) {
        continue;
      }
      const std::string token_text =
          llama_token_to_piece(llama.ctx, token_with_probs.tok);

      size_t pos = std::min(sent_count, llama.generated_text.size());

      const std::string str_test = llama.generated_text.substr(pos);
      bool is_stop_full = false;
      size_t stop_pos =
          llama.findStoppingStrings(str_test, token_text.size(), STOP_FULL);
      if (stop_pos != std::string::npos) {
        is_stop_full = true;
        llama.generated_text.erase(llama.generated_text.begin() + pos +
                                       stop_pos,
                                   llama.generated_text.end());
        pos = std::min(sent_count, llama.generated_text.size());
      } else {
        is_stop_full = false;
        stop_pos = llama.findStoppingStrings(str_test, token_text.size(),
                                             STOP_PARTIAL);
      }

      if (stop_pos == std::string::npos ||
          // Send rest of the text if we are at the end of the generation
          (!llama.has_next_token && !is_stop_full && stop_pos > 0)) {
        const std::string to_send =
            llama.generated_text.substr(pos, std::string::npos);

        sent_count += to_send.size();

        std::vector<completion_token_output> probs_output = {};

        if (llama.params.n_probs > 0) {
          const std::vector<llama_token> to_send_toks =
              llama_tokenize(llama.ctx, to_send, false);
          size_t probs_pos = std::min(sent_token_probs_index,
                                      llama.generated_token_probs.size());
          size_t probs_stop_pos =
              std::min(sent_token_probs_index + to_send_toks.size(),
                       llama.generated_token_probs.size());
          if (probs_pos < probs_stop_pos) {
            probs_output = std::vector<completion_token_output>(
                llama.generated_token_probs.begin() + probs_pos,
                llama.generated_token_probs.begin() + probs_stop_pos);
          }
          sent_token_probs_index = probs_stop_pos;
        }

        const json data = format_partial_response(llama, to_send, probs_output);

        const std::string str =
            "data: " +
            data.dump(-1, ' ', false, json::error_handler_t::replace) + "\n\n";

        LOG_VERBOSE("data stream", {{"to_send", str}});
        std::size_t nRead = std::min(str.size(), nBuffSize);
        memcpy(pBuffer, str.data(), nRead);
        return nRead;
      }

      if (!llama.has_next_token) {
        // Generation is done, send extra information.
        const json data = format_final_response(
            llama, "",
            std::vector<completion_token_output>(
                llama.generated_token_probs.begin(),
                llama.generated_token_probs.begin() + sent_token_probs_index));

        const std::string str =
            "data: " +
            data.dump(-1, ' ', false, json::error_handler_t::replace) + "\n\n";

        LOG_VERBOSE("data stream", {{"to_send", str}});
        std::size_t nRead = std::min(str.size(), nBuffSize);
        memcpy(pBuffer, str.data(), nRead);
        return nRead;
      }
    }
    llama_print_timings(llama.ctx);
    llama.mutex.unlock();
    return 0;
  };

  auto resp = drogon::HttpResponse::newStreamResponse(chunked_content_provider,
                                                      "chat_completions.txt");
  callback(resp);
  return;
}
