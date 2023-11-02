#include "llamaCPP.h"
#include "llama.h"
#include "utils/nitro_utils.h"
#include <chrono>
#include <cstring>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <regex>
#include <thread>
#include <trantor/utils/Logger.h>

using namespace inferences;

std::string create_return_json(const std::string &id, const std::string &model,
                               const std::string &content,
                               Json::Value finish_reason = Json::Value()) {

  Json::Value root;

  root["id"] = id;
  root["model"] = model;
  root["created"] = static_cast<int>(std::time(nullptr));
  root["object"] = "chat.completion.chunk";

  Json::Value choicesArray(Json::arrayValue);
  Json::Value choice;

  choice["index"] = 0;
  Json::Value delta;
  delta["content"] = content;
  choice["delta"] = delta;
  choice["finish_reason"] = finish_reason;

  choicesArray.append(choice);
  root["choices"] = choicesArray;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // This sets the indentation to an empty string,
                              // producing compact output.
  return Json::writeString(writer, root);
}

void llamaCPP::warmupModel() {
  auto lock = llama.lock();
  llama.rewind();
  llama_reset_timings(llama.ctx);

  llama.prompt = "hello";
  llama.params.n_predict = 1;
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
    stop_pos = llama.findStoppingStrings(llama.generated_text, 0, STOP_PARTIAL);
  }
  if (stop_pos != std::string::npos) {
    llama.generated_text.erase(llama.generated_text.begin() + stop_pos,
                               llama.generated_text.end());
  }
  auto probs = llama.generated_token_probs;
  if (llama.params.sampling_params.n_probs > 0 && llama.stopped_word) {
    const std::vector<llama_token> stop_word_toks =
        llama_tokenize(llama.ctx, llama.stopping_word, false);
    probs = std::vector<completion_token_output>(
        llama.generated_token_probs.begin(),
        llama.generated_token_probs.end() - stop_word_toks.size());
  }

  LOG_INFO << "Warm-up generated text:" << llama.generated_text;
  LOG_INFO << "Warm-up finish";
  return;
}

void llamaCPP::chatCompletion(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  if (!model_loaded) {
    Json::Value jsonResp;
    jsonResp["message"] = "Model is not loaded yet";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k500InternalServerError);
    callback(resp);
    return;
  }

  const auto &jsonBody = req->getJsonObject();
  std::string formatted_output =
      "Below is a conversation between an AI system named ASSISTANT and USER\n";
  if (jsonBody) {
    llama.params.n_predict = (*jsonBody)["max_tokens"].asInt();
    llama.params.sampling_params.top_p = (*jsonBody)["top_p"].asFloat();
    llama.params.sampling_params.temp = (*jsonBody)["temperature"].asFloat();
    llama.params.sampling_params.frequency_penalty =
        (*jsonBody)["frequency_penalty"].asFloat();
    llama.params.sampling_params.presence_penalty =
        (*jsonBody)["presence_penalty"].asFloat();

    const Json::Value &messages = (*jsonBody)["messages"];
    for (const auto &message : messages) {
      std::string role = message["role"].asString();
      std::string content = message["content"].asString();
      formatted_output += role + ": " + content + "\n";
    }
    formatted_output += "assistant:";
  }

  this->llama.rewind();

  llama_reset_timings(llama.ctx);

  this->llama.prompt = formatted_output;
  this->llama.params.antiprompt.clear();
  for (const auto &stop_word : (*jsonBody)["stop"]) {
    llama.params.antiprompt.push_back(stop_word.asString());
  }
  this->llama.params.antiprompt.push_back("user:");
  this->llama.params.antiprompt.push_back("### USER:");
  this->llama.loadPrompt();
  this->llama.beginCompletion();

  const auto chunked_content_provider =
      [this](char *pBuffer, std::size_t nBuffSize) -> std::size_t {
    auto lock = this->llama.lock();
    if (!pBuffer) {
      LOG_INFO << "Connection closed or buffer is null. Reset context";
      lock.release();

      llama_print_timings(llama.ctx);
      this->llama.mutex.unlock();
      this->sent_count = 0;
      this->sent_token_probs_index = 0;
      // LOG_INFO << "Test end two time lol";
      return 0;
    }
    // LOG_INFO << this->llama.has_next_token;
    while (this->llama.has_next_token) {
      try {
        // LOG_INFO << this->llama.has_next_token;
        const completion_token_output token_with_probs =
            this->llama.doCompletion();
        if (token_with_probs.tok == -1 || this->llama.multibyte_pending > 0) {
          return 0;
        }
        const std::string token_text =
            llama_token_to_piece(llama.ctx, token_with_probs.tok);

        size_t pos = std::min(sent_count, this->llama.generated_text.size());

        const std::string str_test = this->llama.generated_text.substr(pos);
        bool is_stop_full = false;
        size_t stop_pos = this->llama.findStoppingStrings(
            str_test, token_text.size(), STOP_FULL);
        if (stop_pos != std::string::npos) {
          is_stop_full = true;
          this->llama.generated_text.erase(llama.generated_text.begin() + pos +
                                               stop_pos,
                                           this->llama.generated_text.end());
          pos = std::min(sent_count, this->llama.generated_text.size());
        } else {
          is_stop_full = false;
          stop_pos = this->llama.findStoppingStrings(
              str_test, token_text.size(), STOP_PARTIAL);
        }

        if (stop_pos == std::string::npos ||
            // Send rest of the text if we are at the end of the generation
            (!this->llama.has_next_token && !is_stop_full && stop_pos > 0)) {
          const std::string to_send =
              this->llama.generated_text.substr(pos, std::string::npos);

          sent_count += to_send.size();

          std::vector<completion_token_output> probs_output = {};

          if (this->llama.params.sampling_params.n_probs > 0) {
            const std::vector<llama_token> to_send_toks =
                llama_tokenize(llama.ctx, to_send, false);
            size_t probs_pos =
                std::min(sent_token_probs_index,
                         this->llama.generated_token_probs.size());
            size_t probs_stop_pos =
                std::min(sent_token_probs_index + to_send_toks.size(),
                         this->llama.generated_token_probs.size());
            if (probs_pos < probs_stop_pos) {
              probs_output = std::vector<completion_token_output>(
                  this->llama.generated_token_probs.begin() + probs_pos,
                  this->llama.generated_token_probs.begin() + probs_stop_pos);
            }
            sent_token_probs_index = probs_stop_pos;
          }
          if (!to_send.empty() &&
              llama.has_next_token) { //  NITRO : the patch here is important to
                                      //  make midway cutting possible
            // const json data = format_partial_response(this->llama, to_send,
            // probs_output);
            // LOG_INFO << llama.has_next_token;
            const std::string str =
                "data: " +
                create_return_json(nitro_utils::generate_random_string(20), "_",
                                   to_send) +
                "\n\n";

            LOG_VERBOSE("data stream", {{"to_send", str}});
            std::size_t nRead = std::min(str.size(), nBuffSize);
            memcpy(pBuffer, str.data(), nRead);
            return nRead;
          }
        }

        //       std::this_thread::sleep_for(std::chrono::seconds(2));
        // LOG_INFO << this->llama.has_next_token;
        if (!this->llama.has_next_token) {
          // Generation is done, send extra information.
          //        const json data = format_final_response(
          //            this->llama, "",
          //            std::vector<completion_token_output>(
          //                this->llama.generated_token_probs.begin(),
          //                this->llama.generated_token_probs.begin() +
          //                sent_token_probs_index));
          //

          const std::string str =
              "data: " +
              create_return_json(nitro_utils::generate_random_string(20), "_",
                                 "", "stop") +
              "\n\n" + "data: [DONE]" + "\n\n";

          LOG_VERBOSE("data stream", {{"to_send", str}});
          std::size_t nRead = std::min(str.size(), nBuffSize);
          memcpy(pBuffer, str.data(), nRead);
          return nRead;
        }
      } catch (...) {
        LOG_ERROR << "error inside while loop";
      }
    }
    lock.release();

    llama_print_timings(llama.ctx);
    this->llama.mutex.unlock();
    this->sent_count = 0;
    this->sent_token_probs_index = 0;
    // LOG_INFO << "Test end two time lol";
    return 0;
  };

  auto resp = nitro_utils::nitroStreamResponse(chunked_content_provider,
                                               "chat_completions.txt");
  callback(resp);
}

void llamaCPP::embedding(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  if (!model_loaded) {
    Json::Value jsonResp;
    jsonResp["message"] = "Model is not loaded yet";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k500InternalServerError);
    callback(resp);
    return;
  }

  auto lock = llama.lock();

  const auto &jsonBody = req->getJsonObject();

  llama.rewind();
  llama_reset_timings(llama.ctx);
  if (jsonBody->isMember("content") != 0) {
    llama.prompt = (*jsonBody)["content"].asString();
  } else {
    llama.prompt = "";
  }
  llama.params.n_predict = 0;
  llama.loadPrompt();
  llama.beginCompletion();
  llama.doCompletion();

  const json data = format_embedding_response(llama);
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(data.dump());
  resp->setContentTypeString("application/json");
  callback(resp);
}

void llamaCPP::loadModel(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

  const auto &jsonBody = req->getJsonObject();

  gpt_params params;
  if (jsonBody) {
    params.model = (*jsonBody)["llama_model_path"].asString();
    params.n_gpu_layers = (*jsonBody)["ngl"].asInt();
    params.n_ctx = (*jsonBody)["ctx_len"].asInt();
    params.embedding = (*jsonBody)["embedding"].asBool();
  }
#ifdef GGML_USE_CUBLAS
  LOG_INFO << "Setting up GGML CUBLAS PARAMS";
  params.mul_mat_q = false;
#endif // GGML_USE_CUBLAS
  if (params.model_alias == "unknown") {
    params.model_alias = params.model;
  }

  llama_backend_init(params.numa);

  LOG_INFO_LLAMA("build info",
                 {{"build", BUILD_NUMBER}, {"commit", BUILD_COMMIT}});
  LOG_INFO_LLAMA("system info",
                 {
                     {"n_threads", params.n_threads},
                     {"total_threads", std::thread::hardware_concurrency()},
                     {"system_info", llama_print_system_info()},
                 });

  // load the model
  if (!llama.loadModel(params)) {
    LOG_ERROR << "Error loading the model will exit the program";
    Json::Value jsonResp;
    jsonResp["message"] = "Model loaded failed";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k500InternalServerError);
    callback(resp);
  }
  Json::Value jsonResp;
  jsonResp["message"] = "Model loaded successfully";
  model_loaded = true;
  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  warmupModel();
  callback(resp);
}

void inferences::llamaCPP::unloadModel(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback)
{
  Json::Value jsonResp;
  jsonResp["message"] = "No model loaded";
  if (model_loaded) {
    llama.unloadModel();
    model_loaded = false;
    jsonResp["message"] = "Model unloaded successfully";
  }
  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(resp);
}
