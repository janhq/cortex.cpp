#include "llamaCPP.h"
#include "llama.h"
#include "log.h"
#include "utils/nitro_utils.h"

using namespace inferences;
using json = nlohmann::json;

struct inferenceState {
  bool is_stopped = false;
  bool is_streaming = false;
  int task_id;
  llamaCPP *instance;

  inferenceState(llamaCPP *inst) : instance(inst) {}
};

std::shared_ptr<inferenceState> create_inference_state(llamaCPP *instance) {
  return std::make_shared<inferenceState>(instance);
}

// --------------------------------------------

std::string create_embedding_payload(const std::vector<float> &embedding,
                                     int prompt_tokens) {
  Json::Value root;

  root["object"] = "list";

  Json::Value dataArray(Json::arrayValue);
  Json::Value dataItem;

  dataItem["object"] = "embedding";

  Json::Value embeddingArray(Json::arrayValue);
  for (const auto &value : embedding) {
    embeddingArray.append(value);
  }
  dataItem["embedding"] = embeddingArray;
  dataItem["index"] = 0;

  dataArray.append(dataItem);
  root["data"] = dataArray;

  root["model"] = "_";

  Json::Value usage;
  usage["prompt_tokens"] = prompt_tokens;
  usage["total_tokens"] = prompt_tokens; // Assuming total tokens equals prompt
                                         // tokens in this context
  root["usage"] = usage;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // Compact output
  return Json::writeString(writer, root);
}

std::string create_full_return_json(const std::string &id,
                                    const std::string &model,
                                    const std::string &content,
                                    const std::string &system_fingerprint,
                                    int prompt_tokens, int completion_tokens,
                                    Json::Value finish_reason = Json::Value()) {

  Json::Value root;

  root["id"] = id;
  root["model"] = model;
  root["created"] = static_cast<int>(std::time(nullptr));
  root["object"] = "chat.completion";
  root["system_fingerprint"] = system_fingerprint;

  Json::Value choicesArray(Json::arrayValue);
  Json::Value choice;

  choice["index"] = 0;
  Json::Value message;
  message["role"] = "assistant";
  message["content"] = content;
  choice["message"] = message;
  choice["finish_reason"] = finish_reason;

  choicesArray.append(choice);
  root["choices"] = choicesArray;

  Json::Value usage;
  usage["prompt_tokens"] = prompt_tokens;
  usage["completion_tokens"] = completion_tokens;
  usage["total_tokens"] = prompt_tokens + completion_tokens;
  root["usage"] = usage;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // Compact output
  return Json::writeString(writer, root);
}

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
  json pseudo;

  pseudo["prompt"] = "Hello";
  pseudo["n_predict"] = 2;
  pseudo["stream"] = false;
  const int task_id = llama.request_completion(pseudo, false, false, -1);
  std::string completion_text;
  task_result result = llama.next_result(task_id);
  if (!result.error && result.stop) {
    LOG_INFO << result.result_json.dump(-1, ' ', false,
                                        json::error_handler_t::replace);
  }
  return;
}

void llamaCPP::chatCompletionPrelight(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setStatusCode(drogon::HttpStatusCode::k200OK);
  resp->addHeader("Access-Control-Allow-Origin", "*");
  resp->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  resp->addHeader("Access-Control-Allow-Headers", "*");
  callback(resp);
}

void llamaCPP::chatCompletion(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

  if (!llama.model_loaded_external) {
    Json::Value jsonResp;
    jsonResp["message"] =
        "Model has not been loaded, please load model into nitro";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k409Conflict);
    callback(resp);
  }

  const auto &jsonBody = req->getJsonObject();
  std::string formatted_output = pre_prompt;

  json data;
  json stopWords;
  int no_images = 0;
  // To set default value

  if (jsonBody) {
    // Increase number of chats received and clean the prompt
    no_of_chats++;
    if (no_of_chats % clean_cache_threshold == 0) {
      LOG_INFO << "Clean cache threshold reached!";
      llama.kv_cache_clear();
      LOG_INFO << "Cache cleaned";
    }

    // Default values to enable auto caching
    data["cache_prompt"] = caching_enabled;
    data["n_keep"] = -1;

    // Passing load value
    data["repeat_last_n"] = this->repeat_last_n;

    data["stream"] = (*jsonBody).get("stream", false).asBool();
    data["n_predict"] = (*jsonBody).get("max_tokens", 500).asInt();
    data["top_p"] = (*jsonBody).get("top_p", 0.95).asFloat();
    data["temperature"] = (*jsonBody).get("temperature", 0.8).asFloat();
    data["frequency_penalty"] =
        (*jsonBody).get("frequency_penalty", 0).asFloat();
    data["presence_penalty"] = (*jsonBody).get("presence_penalty", 0).asFloat();
    const Json::Value &messages = (*jsonBody)["messages"];
    std::string grammar_file = (*jsonBody).get("grammar_file", "").asString();
    std::ifstream file(grammar_file);
    if (!file) {
      LOG_ERROR << "Grammar file not found";
    } else {
      std::stringstream grammarBuf;
      grammarBuf << file.rdbuf();
      data["grammar"] = grammarBuf.str();
    }
    if (!llama.multimodal) {

      for (const auto &message : messages) {
        std::string input_role = message["role"].asString();
        std::string role;
        if (input_role == "user") {
          role = user_prompt;
          std::string content = message["content"].asString();
          formatted_output += role + content;
        } else if (input_role == "assistant") {
          role = ai_prompt;
          std::string content = message["content"].asString();
          formatted_output += role + content;
        } else if (input_role == "system") {
          role = system_prompt;
          std::string content = message["content"].asString();
          formatted_output = role + content + formatted_output;

        } else {
          role = input_role;
          std::string content = message["content"].asString();
          formatted_output += role + content;
        }
      }
      formatted_output += ai_prompt;
    } else {

      data["image_data"] = json::array();
      for (const auto &message : messages) {
        std::string input_role = message["role"].asString();
        std::string role;
        if (input_role == "user") {
          formatted_output += role;
          for (auto content_piece : message["content"]) {
            role = user_prompt;

            auto content_piece_type = content_piece["type"].asString();
            if (content_piece_type == "text") {
              auto text = content_piece["text"].asString();
              formatted_output += text;
            } else if (content_piece_type == "image_url") {
              auto image_url = content_piece["image_url"]["url"].asString();
              auto base64_image_data = nitro_utils::extractBase64(image_url);
              LOG_INFO << base64_image_data;
              formatted_output += "[img-" + std::to_string(no_images) + "]";

              json content_piece_image_data;
              content_piece_image_data["data"] = base64_image_data;
              content_piece_image_data["id"] = no_images;
              data["image_data"].push_back(content_piece_image_data);
              no_images++;
            }
          }

        } else if (input_role == "assistant") {
          role = ai_prompt;
          std::string content = message["content"].asString();
          formatted_output += role + content;
        } else if (input_role == "system") {
          role = system_prompt;
          std::string content = message["content"].asString();
          formatted_output = role + content + formatted_output;

        } else {
          role = input_role;
          std::string content = message["content"].asString();
          formatted_output += role + content;
        }
      }
      formatted_output += ai_prompt;
      LOG_INFO << formatted_output;
    }

    data["prompt"] = formatted_output;
    for (const auto &stop_word : (*jsonBody)["stop"]) {
      stopWords.push_back(stop_word.asString());
    }
    // specify default stop words
    // Ensure success case for chatML
    stopWords.push_back("<|im_end|>");
    stopWords.push_back(nitro_utils::rtrim(user_prompt));
    data["stop"] = stopWords;
  }

  bool is_streamed = data["stream"];
// Enable full message debugging
#ifdef DEBUG
  LOG_INFO << "Current completion text";
  LOG_INFO << formatted_output;
#endif
  int task_id;

  LOG_INFO << "Resolved request for task_id:" << task_id;

  if (is_streamed) {
    auto state = create_inference_state(this);
    state->task_id = task_id;
    auto chunked_content_provider =
        [state, data](char *pBuffer, std::size_t nBuffSize) -> std::size_t {
      if (!state->is_streaming) {
        state->task_id =
            state->instance->llama.request_completion(data, false, false, -1);
        state->instance->single_queue_is_busy = true;
      }
      if (!pBuffer) {
        LOG_INFO << "Connection closed or buffer is null. Reset context";
        state->instance->llama.request_cancel(state->task_id);
        state->is_streaming = false;
        state->instance->single_queue_is_busy = false;
        return 0;
      }
      if (state->is_stopped) {
        state->is_streaming = false;
        state->instance->single_queue_is_busy = false;
        return 0;
      }

      task_result result = state->instance->llama.next_result(state->task_id);
      if (!result.error) {
        // Update streaming state to being streamed
        state->is_streaming = true;
        const std::string to_send = result.result_json["content"];
        const std::string str =
            "data: " +
            create_return_json(nitro_utils::generate_random_string(20), "_",
                               to_send) +
            "\n\n";

        std::size_t nRead = std::min(str.size(), nBuffSize);
        memcpy(pBuffer, str.data(), nRead);

        if (result.stop) {
          const std::string str =
              "data: " +
              create_return_json(nitro_utils::generate_random_string(20), "_",
                                 "", "stop") +
              "\n\n" + "data: [DONE]" + "\n\n";

          LOG_VERBOSE("data stream", {{"to_send", str}});
          std::size_t nRead = std::min(str.size(), nBuffSize);
          memcpy(pBuffer, str.data(), nRead);
          LOG_INFO << "reached result stop";
          state->is_stopped = true;
          state->instance->llama.request_cancel(state->task_id);
          state->is_streaming = false;
          state->instance->single_queue_is_busy = false;

          return nRead;
        }
        return nRead;
      } else {
        if (state->instance->llama.params.n_parallel == 1) {
          while (state->instance->single_queue_is_busy) {
            LOG_INFO << "Waiting for task to be released status:"
                     << state->instance->single_queue_is_busy;
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Waiting in 500 miliseconds step
          }
        }
        std::string str = "\n\n";
        std::size_t nRead = str.size();
        memcpy(pBuffer, str.data(), nRead);
        LOG_INFO << "Failing retrying now";
        return nRead;
      }
      state->is_streaming = false;
      state->instance->single_queue_is_busy = false;
      return 0;
    };
    auto resp = nitro_utils::nitroStreamResponse(chunked_content_provider,
                                                 "chat_completions.txt");
    callback(resp);

    return;
  } else {
    Json::Value respData;
    auto resp = nitro_utils::nitroHttpResponse();
    if (!json_value(data, "stream", false)) {
      std::string completion_text;
      task_result result = llama.next_result(task_id);
      if (!result.error && result.stop) {
        int prompt_tokens = result.result_json["tokens_evaluated"];
        int predicted_tokens = result.result_json["tokens_predicted"];
        std::string full_return =
            create_full_return_json(nitro_utils::generate_random_string(20),
                                    "_", result.result_json["content"], "_",
                                    prompt_tokens, predicted_tokens);
        resp->setBody(full_return);
      } else {
        resp->setBody("internal error during inference");
        return;
      }
      callback(resp);
      return;
    }
  }
}
void llamaCPP::embedding(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  const auto &jsonBody = req->getJsonObject();

  json prompt;
  if (jsonBody->isMember("input") != 0) {
    prompt = (*jsonBody)["input"].asString();
  } else {
    prompt = "";
  }
  const int task_id = llama.request_completion(
      {{"prompt", prompt}, {"n_predict", 0}}, false, true, -1);
  task_result result = llama.next_result(task_id);
  std::vector<float> embedding_result = result.result_json["embedding"];
  auto resp = nitro_utils::nitroHttpResponse();
  std::string embedding_resp = create_embedding_payload(embedding_result, 0);
  resp->setBody(embedding_resp);
  resp->setContentTypeString("application/json");
  callback(resp);
  return;
}

void llamaCPP::unloadModel(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  Json::Value jsonResp;
  jsonResp["message"] = "No model loaded";
  if (llama.model_loaded_external) {
    stopBackgroundTask();

    llama_free(llama.ctx);
    llama_free_model(llama.model);
    llama.ctx = nullptr;
    llama.model = nullptr;
    jsonResp["message"] = "Model unloaded successfully";
  }
  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(resp);
  return;
}
void llamaCPP::modelStatus(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  Json::Value jsonResp;
  bool is_model_loaded = llama.model_loaded_external;
  if (is_model_loaded) {
    jsonResp["model_loaded"] = is_model_loaded;
    jsonResp["model_data"] = llama.get_model_props().dump();
  } else {
    jsonResp["model_loaded"] = is_model_loaded;
  }

  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(resp);
  return;
}

bool llamaCPP::loadModelImpl(const Json::Value &jsonBody) {

  gpt_params params;

  // By default will setting based on number of handlers
  if (jsonBody) {
    if (!jsonBody["mmproj"].isNull()) {
      LOG_INFO << "MMPROJ FILE detected, multi-model enabled!";
      params.mmproj = jsonBody["mmproj"].asString();
    }
    if (!jsonBody["grp_attn_n"].isNull()) {

      params.grp_attn_n = jsonBody["grp_attn_n"].asInt();
    }
    if (!jsonBody["grp_attn_w"].isNull()) {

      params.grp_attn_w = jsonBody["grp_attn_w"].asInt();
    }
    params.model = jsonBody["llama_model_path"].asString();
    params.n_gpu_layers = jsonBody.get("ngl", 100).asInt();
    params.n_ctx = jsonBody.get("ctx_len", 2048).asInt();
    params.embedding = jsonBody.get("embedding", true).asBool();
    // Check if n_parallel exists in jsonBody, if not, set to drogon_thread
    params.n_batch = jsonBody.get("n_batch", 512).asInt();
    params.n_parallel = jsonBody.get("n_parallel", 1).asInt();
    params.n_threads =
        jsonBody.get("cpu_threads", std::thread::hardware_concurrency())
            .asInt();
    params.cont_batching = jsonBody.get("cont_batching", false).asBool();
    this->clean_cache_threshold =
        jsonBody.get("clean_cache_threshold", 5).asInt();
    this->caching_enabled = jsonBody.get("caching_enabled", false).asBool();
    this->user_prompt = jsonBody.get("user_prompt", "USER: ").asString();
    this->ai_prompt = jsonBody.get("ai_prompt", "ASSISTANT: ").asString();
    this->system_prompt =
        jsonBody.get("system_prompt", "ASSISTANT's RULE: ").asString();
    this->pre_prompt = jsonBody.get("pre_prompt", "").asString();
    this->repeat_last_n = jsonBody.get("repeat_last_n", 32).asInt();

    if (!jsonBody["llama_log_folder"].isNull()) {
      log_enable();
      std::string llama_log_folder = jsonBody["llama_log_folder"].asString();
      log_set_target(llama_log_folder + "llama.log");
    } // Set folder for llama log
  }
#ifdef GGML_USE_CUBLAS
  LOG_INFO << "Setting up GGML CUBLAS PARAMS";
  params.mul_mat_q = false;
#endif // GGML_USE_CUBLAS
  if (params.model_alias == "unknown") {
    params.model_alias = params.model;
  }

  llama_backend_init(params.numa);

  // LOG_INFO_LLAMA("build info",
  //                {{"build", BUILD_NUMBER}, {"commit", BUILD_COMMIT}});
  LOG_INFO_LLAMA("system info",
                 {
                     {"n_threads", params.n_threads},
                     {"total_threads", std::thread::hardware_concurrency()},
                     {"system_info", llama_print_system_info()},
                 });

  // load the model
  if (!llama.load_model(params)) {
    LOG_ERROR << "Error loading the model";
    return false; // Indicate failure
  }
  llama.initialize();

  llama.model_loaded_external = true;

  LOG_INFO << "Started background task here!";
  backgroundThread = std::thread(&llamaCPP::backgroundTask, this);
  warmupModel();
  return true;
}

void llamaCPP::loadModel(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

  if (llama.model_loaded_external) {
    LOG_INFO << "model loaded";
    Json::Value jsonResp;
    jsonResp["message"] = "Model already loaded";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k409Conflict);
    callback(resp);
    return;
  }

  const auto &jsonBody = req->getJsonObject();
  if (!loadModelImpl(*jsonBody)) {
    // Error occurred during model loading
    Json::Value jsonResp;
    jsonResp["message"] = "Failed to load model";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k500InternalServerError);
    callback(resp);
  } else {
    // Model loaded successfully
    Json::Value jsonResp;
    jsonResp["message"] = "Model loaded successfully";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(resp);
  }
}

void llamaCPP::backgroundTask() {
  while (llama.model_loaded_external) {
    // model_loaded =
    llama.update_slots();
  }
  LOG_INFO << "Background task stopped! ";
  llama.kv_cache_clear();
  LOG_INFO << "KV cache cleared!";
  return;
}

void llamaCPP::stopBackgroundTask() {
  if (llama.model_loaded_external) {
    llama.model_loaded_external = false;
    llama.condition_tasks.notify_one();
    LOG_INFO << "changed to false";
    if (backgroundThread.joinable()) {
      backgroundThread.join();
    }
  }
}
