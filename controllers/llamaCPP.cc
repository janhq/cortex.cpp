#include "llamaCPP.h"

#include <fstream>
#include <iostream>
#include "log.h"
#include "utils/logging_utils.h"
#include "utils/nitro_utils.h"

// External
#include "common.h"
#include "llama.h"

using namespace inferences;
using json = nlohmann::json;

/**
 * The state of the inference task
 */
enum InferenceStatus { PENDING, RUNNING, EOS, FINISHED };

/**
 * There is a need to save state of current ongoing inference status of a
 * handler, this struct is to solve that issue
 *
 * @param inst Pointer to the llamaCPP instance this inference task is
 * associated with.
 */
struct inferenceState {
  int task_id;
  InferenceStatus inference_status = PENDING;
  llamaCPP* instance;
  // Check if we receive the first token, set it to false after receiving
  bool is_first_token = true;

  inferenceState(llamaCPP* inst) : instance(inst) {}
};

/**
 * This function is to create the smart pointer to inferenceState, hence the
 * inferenceState will be persisting even tho the lambda in streaming might go
 * out of scope and the handler already moved on
 */
std::shared_ptr<inferenceState> create_inference_state(llamaCPP* instance) {
  return std::make_shared<inferenceState>(instance);
}

/**
 * Check if model already loaded if not return message to user
 * @param callback the function to return message to user
 */
bool llamaCPP::CheckModelLoaded(
    const std::function<void(const HttpResponsePtr&)>& callback) {
  if (!llama.model_loaded_external) {
    LOG_ERROR << "Model has not been loaded";
    Json::Value jsonResp;
    jsonResp["message"] =
        "Model has not been loaded, please load model into nitro";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k409Conflict);
    callback(resp);
    return false;
  }
  return true;
}

Json::Value create_embedding_payload(const std::vector<float>& embedding,
                                     int prompt_tokens) {
  Json::Value dataItem;

  dataItem["object"] = "embedding";

  Json::Value embeddingArray(Json::arrayValue);
  for (const auto& value : embedding) {
    embeddingArray.append(value);
  }
  dataItem["embedding"] = embeddingArray;
  dataItem["index"] = 0;

  return dataItem;
}

Json::Value create_full_return_json(const std::string& id,
                                    const std::string& model,
                                    const std::string& content,
                                    const std::string& system_fingerprint,
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

  return root;
}

std::string create_return_json(const std::string& id, const std::string& model,
                               const std::string& content,
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
  writer["indentation"] = "";  // This sets the indentation to an empty string,
                               // producing compact output.
  return Json::writeString(writer, root);
}

llamaCPP::llamaCPP()
    : queue(new trantor::ConcurrentTaskQueue(llama.params.n_parallel,
                                             "llamaCPP")) {
  // Some default values for now below
  log_disable();  // Disable the log to file feature, reduce bloat for
                  // target
                  // system ()
};

llamaCPP::~llamaCPP() {
  StopBackgroundTask();
}

void llamaCPP::WarmupModel() {
  json pseudo;

  LOG_INFO << "Warm-up model";
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

void llamaCPP::ChatCompletion(
    inferences::ChatCompletionRequest&& completion,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  // Check if model is loaded
  if (CheckModelLoaded(callback)) {
    // Model is loaded
    // Do Inference
    InferenceImpl(std::move(completion), std::move(callback));
  }
}

void llamaCPP::InferenceImpl(
    inferences::ChatCompletionRequest&& completion,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (llama.model_type == ModelType::EMBEDDING) {
    LOG_WARN << "Not support completion for embedding model";
    Json::Value jsonResp;
    jsonResp["message"] = "Not support completion for embedding model";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
    return;
  }
  std::string formatted_output = pre_prompt;
  int request_id = ++no_of_requests;
  LOG_INFO_REQUEST(request_id) << "Generating reponse for inference request";

  json data;
  json stopWords;
  int no_images = 0;
  // To set default value

  // Increase number of chats received and clean the prompt
  //no_of_chats++;
  //if (no_of_chats % clean_cache_threshold == 0) {
  //  LOG_INFO_REQUEST(request_id) << "Clean cache threshold reached!";
  //  llama.kv_cache_clear();
  //  LOG_INFO_REQUEST(request_id) << "Cache cleaned";
  //}

  // Default values to enable auto caching
  //data["cache_prompt"] = caching_enabled;
  data["cache_prompt"] = false;
  data["n_keep"] = -1;

  // Passing load value
  data["repeat_last_n"] = this->repeat_last_n;
  LOG_INFO_REQUEST(request_id)
      << "Stop words:" << completion.stop.toStyledString();

  data["stream"] = completion.stream;
  data["n_predict"] = completion.max_tokens;
  data["top_p"] = completion.top_p;
  data["temperature"] = completion.temperature;
  data["frequency_penalty"] = completion.frequency_penalty;
  data["presence_penalty"] = completion.presence_penalty;
  const Json::Value& messages = completion.messages;

  if (!grammar_file_content.empty()) {
    data["grammar"] = grammar_file_content;
  };

  if (!llama.multimodal) {
    for (const auto& message : messages) {
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
    for (const auto& message : messages) {
      std::string input_role = message["role"].asString();
      std::string role;
      if (input_role == "user") {
        formatted_output += role;
        for (auto content_piece : message["content"]) {
          role = user_prompt;

          json content_piece_image_data;
          content_piece_image_data["data"] = "";

          auto content_piece_type = content_piece["type"].asString();
          if (content_piece_type == "text") {
            auto text = content_piece["text"].asString();
            formatted_output += text;
          } else if (content_piece_type == "image_url") {
            auto image_url = content_piece["image_url"]["url"].asString();
            std::string base64_image_data;
            if (image_url.find("http") != std::string::npos) {
              LOG_INFO_REQUEST(request_id)
                  << "Remote image detected but not supported yet";
            } else if (image_url.find("data:image") != std::string::npos) {
              LOG_INFO_REQUEST(request_id) << "Base64 image detected";
              base64_image_data = nitro_utils::extractBase64(image_url);
              LOG_INFO_REQUEST(request_id) << base64_image_data;
            } else {
              LOG_INFO_REQUEST(request_id) << "Local image detected";
              nitro_utils::processLocalImage(
                  image_url, [&](const std::string& base64Image) {
                    base64_image_data = base64Image;
                  });
              LOG_INFO_REQUEST(request_id) << base64_image_data;
            }
            content_piece_image_data["data"] = base64_image_data;

            formatted_output += "[img-" + std::to_string(no_images) + "]";
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
    LOG_INFO_REQUEST(request_id) << formatted_output;
  }

  data["prompt"] = formatted_output;
  for (const auto& stop_word : completion.stop) {
    stopWords.push_back(stop_word.asString());
  }
  // specify default stop words
  // Ensure success case for chatML
  stopWords.push_back("<|im_end|>");
  stopWords.push_back(nitro_utils::rtrim(user_prompt));
  data["stop"] = stopWords;

  bool is_streamed = data["stream"];
// Enable full message debugging
#ifdef DEBUG
  LOG_INFO_REQUEST(request_id) << "Current completion text";
  LOG_INFO_REQUEST(request_id) << formatted_output;
#endif

  if (is_streamed) {
    LOG_INFO_REQUEST(request_id) << "Streamed, waiting for respone";
    auto state = create_inference_state(this);

    auto chunked_content_provider = [state, data, request_id](
                                        char* pBuffer,
                                        std::size_t nBuffSize) -> std::size_t {
      if (state->inference_status == PENDING) {
        state->inference_status = RUNNING;
      } else if (state->inference_status == FINISHED) {
        return 0;
      }

      if (!pBuffer) {
        LOG_WARN_REQUEST(request_id)
        "Connection closed or buffer is null. Reset context";
        state->inference_status = FINISHED;
        return 0;
      }

      if (state->inference_status == EOS) {
        LOG_INFO_REQUEST(request_id) << "End of result";
        const std::string str =
            "data: " +
            create_return_json(nitro_utils::generate_random_string(20), "_", "",
                               "stop") +
            "\n\n" + "data: [DONE]" + "\n\n";

        LOG_VERBOSE("data stream",
                    {{"request_id": request_id}, {"to_send", str}});
        std::size_t nRead = std::min(str.size(), nBuffSize);
        memcpy(pBuffer, str.data(), nRead);
        state->inference_status = FINISHED;
        return nRead;
      }

      task_result result = state->instance->llama.next_result(state->task_id);
      if (!result.error) {
        std::string to_send = result.result_json["content"];

        // trim the leading space if it is the first token
        if (std::exchange(state->is_first_token, false)) {
          nitro_utils::ltrim(to_send);
        }

        const std::string str =
            "data: " +
            create_return_json(nitro_utils::generate_random_string(20), "_",
                               to_send) +
            "\n\n";

        std::size_t nRead = std::min(str.size(), nBuffSize);
        memcpy(pBuffer, str.data(), nRead);

        if (result.stop) {
          LOG_INFO_REQUEST(request_id) << "Reached result stop";
          state->inference_status = EOS;
          return nRead;
        }

        // Make sure nBufferSize is not zero
        // Otherwise it stop streaming
        if (!nRead) {
          state->inference_status = FINISHED;
        }

        return nRead;
      } else {
        LOG_ERROR_REQUEST(request_id) << "Error during inference";
      }
      state->inference_status = FINISHED;
      return 0;
    };
    // Queued task
    state->instance->queue->runTaskInQueue([cb = std::move(callback), state,
                                            data, chunked_content_provider,
                                            request_id]() {
      state->task_id =
          state->instance->llama.request_completion(data, false, false, -1);

      // Start streaming response
      auto resp = nitro_utils::nitroStreamResponse(chunked_content_provider,
                                                   "chat_completions.txt");
      cb(resp);

      int retries = 0;

      // Since this is an async task, we will wait for the task to be
      // completed
      while (state->inference_status != FINISHED && retries < 10 &&
             state->instance->llama.model_loaded_external) {
        // Should wait chunked_content_provider lambda to be called within
        // 3s
        if (state->inference_status == PENDING) {
          retries += 1;
        }
        if (state->inference_status != RUNNING)
          LOG_INFO_REQUEST(request_id)
              << "Wait for task to be released:" << state->task_id;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      LOG_INFO_REQUEST(request_id) << "Task completed, release it";
      // Request completed, release it
      state->instance->llama.request_cancel(state->task_id);
      LOG_INFO_REQUEST(request_id) << "Inference completed";
    });
  } else {
    queue->runTaskInQueue(
        [this, request_id, cb = std::move(callback), d = std::move(data)]() {
          Json::Value respData;
          int task_id = llama.request_completion(d, false, false, -1);
          LOG_INFO_REQUEST(request_id) << "Non stream, waiting for respone";
          if (!json_value(d, "stream", false)) {
            std::string completion_text;
            task_result result = llama.next_result(task_id);
            if (!result.error && result.stop) {
              int prompt_tokens = result.result_json["tokens_evaluated"];
              int predicted_tokens = result.result_json["tokens_predicted"];
              std::string to_send = result.result_json["content"];
              nitro_utils::ltrim(to_send);
              respData = create_full_return_json(
                  nitro_utils::generate_random_string(20), "_", to_send, "_",
                  prompt_tokens, predicted_tokens);
            } else {
              respData["message"] = "Internal error during inference";
              LOG_ERROR_REQUEST(request_id) << "Error during inference";
            }
            auto resp = nitro_utils::nitroHttpJsonResponse(respData);
            cb(resp);
            LOG_INFO_REQUEST(request_id) << "Inference completed";
          }
        });
  }
}

void llamaCPP::Embedding(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  // Check if model is loaded
  if (CheckModelLoaded(callback)) {
    // Model is loaded
    const auto& jsonBody = req->getJsonObject();
    // Run embedding
    EmbeddingImpl(jsonBody, std::move(callback));
    return;
  }
}

void llamaCPP::EmbeddingImpl(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  int request_id = ++no_of_requests;
  LOG_INFO_REQUEST(request_id) << "Generating reponse for embedding request";
  // Queue embedding task
  auto state = create_inference_state(this);

  state->instance->queue->runTaskInQueue([this, state, jsonBody, callback,
                                          request_id]() {
    Json::Value responseData(Json::arrayValue);

    if (jsonBody->isMember("input")) {
      const Json::Value& input = (*jsonBody)["input"];
      if (input.isString()) {
        // Process the single string input
        state->task_id = llama.request_completion(
            {{"prompt", input.asString()}, {"n_predict", 0}}, false, true, -1);
        task_result result = llama.next_result(state->task_id);
        std::vector<float> embedding_result = result.result_json["embedding"];
        responseData.append(create_embedding_payload(embedding_result, 0));
      } else if (input.isArray()) {
        // Process each element in the array input
        for (const auto& elem : input) {
          if (elem.isString()) {
            const int task_id = llama.request_completion(
                {{"prompt", elem.asString()}, {"n_predict", 0}}, false, true,
                -1);
            task_result result = llama.next_result(task_id);
            std::vector<float> embedding_result =
                result.result_json["embedding"];
            responseData.append(create_embedding_payload(embedding_result, 0));
          }
        }
      }
    }

    Json::Value root;
    root["data"] = responseData;
    root["model"] = "_";
    root["object"] = "list";
    Json::Value usage;
    usage["prompt_tokens"] = 0;
    usage["total_tokens"] = 0;
    root["usage"] = usage;

    auto resp = nitro_utils::nitroHttpJsonResponse(root);
    callback(resp);
    LOG_INFO_REQUEST(request_id) << "Embedding completed";
  });
}

void llamaCPP::UnloadModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  Json::Value jsonResp;
  if (CheckModelLoaded(callback)) {
    StopBackgroundTask();

    llama_free(llama.ctx);
    llama_free_model(llama.model);
    llama.ctx = nullptr;
    llama.model = nullptr;
    jsonResp["message"] = "Model unloaded successfully";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(resp);
    LOG_INFO << "Model unloaded successfully";
  }
}

void llamaCPP::ModelStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  Json::Value jsonResp;
  bool is_model_loaded = llama.model_loaded_external;
  if (CheckModelLoaded(callback)) {
    jsonResp["model_loaded"] = is_model_loaded;
    jsonResp["model_data"] = llama.get_model_props().dump();
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    callback(resp);
    LOG_INFO << "Model status responded";
  }
}

void llamaCPP::LoadModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

  if (!nitro_utils::isAVX2Supported() && ggml_cpu_has_avx2()) {
    LOG_ERROR << "AVX2 is not supported by your processor";
    Json::Value jsonResp;
    jsonResp["message"] =
        "AVX2 is not supported by your processor, please download and replace "
        "the correct Nitro asset version";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k500InternalServerError);
    callback(resp);
    return;
  }

  if (llama.model_loaded_external) {
    LOG_INFO << "Model already loaded";
    Json::Value jsonResp;
    jsonResp["message"] = "Model already loaded";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k409Conflict);
    callback(resp);
    return;
  }

  const auto& jsonBody = req->getJsonObject();
  if (!LoadModelImpl(jsonBody)) {
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
    LOG_INFO << "Model loaded successfully";
  }
}

bool llamaCPP::LoadModelImpl(std::shared_ptr<Json::Value> jsonBody) {
  gpt_params params;
  std::string model_type;
  // By default will setting based on number of handlers
  if (jsonBody) {
    if (!jsonBody->operator[]("mmproj").isNull()) {
      LOG_INFO << "MMPROJ FILE detected, multi-model enabled!";
      params.mmproj = jsonBody->operator[]("mmproj").asString();
    }
    if (!jsonBody->operator[]("grp_attn_n").isNull()) {
      params.grp_attn_n = jsonBody->operator[]("grp_attn_n").asInt();
    }
    if (!jsonBody->operator[]("grp_attn_w").isNull()) {
      params.grp_attn_w = jsonBody->operator[]("grp_attn_w").asInt();
    }
    if (!jsonBody->operator[]("mlock").isNull()) {
      params.use_mlock = jsonBody->operator[]("mlock").asBool();
    }

    if (!jsonBody->operator[]("grammar_file").isNull()) {
      std::string grammar_file =
          jsonBody->operator[]("grammar_file").asString();
      std::ifstream file(grammar_file);
      if (!file) {
        LOG_ERROR << "Grammar file not found";
      } else {
        std::stringstream grammarBuf;
        grammarBuf << file.rdbuf();
        grammar_file_content = grammarBuf.str();
      }
    };

    Json::Value model_path = jsonBody->operator[]("llama_model_path");
    if (model_path.isNull()) {
      LOG_ERROR << "Missing model path in request";
    } else {
      if (std::filesystem::exists(
              std::filesystem::path(model_path.asString()))) {
        params.model = model_path.asString();
      } else {
        LOG_ERROR << "Could not find model in path " << model_path.asString();
      }
    }

#if defined(__APPLE__) && !defined(GGML_USE_METAL)
  params.n_gpu_layers = 0;
  fprintf(stderr, "Cameron1: params.n_gpu_layers = 0");
#else
  params.n_gpu_layers = jsonBody->get("ngl", 100).asInt();
  fprintf(stderr, "Cameron2: params.n_gpu_layers = 100");
#endif

    params.n_ctx = jsonBody->get("ctx_len", 2048).asInt();
    params.embedding = jsonBody->get("embedding", true).asBool();
    model_type = jsonBody->get("model_type", "llm").asString();
    if (model_type == "llm") {
      llama.model_type = ModelType::LLM;
    } else {
      llama.model_type = ModelType::EMBEDDING;
    }
    // Check if n_parallel exists in jsonBody, if not, set to drogon_thread
    params.n_batch = jsonBody->get("n_batch", 512).asInt();
    params.n_parallel = jsonBody->get("n_parallel", 1).asInt();
    params.n_threads =
        jsonBody->get("cpu_threads", std::thread::hardware_concurrency())
            .asInt();
    params.cont_batching = jsonBody->get("cont_batching", false).asBool();
    this->clean_cache_threshold =
        jsonBody->get("clean_cache_threshold", 5).asInt();
    this->caching_enabled = jsonBody->get("caching_enabled", false).asBool();
    this->user_prompt = jsonBody->get("user_prompt", "USER: ").asString();
    this->ai_prompt = jsonBody->get("ai_prompt", "ASSISTANT: ").asString();
    this->system_prompt =
        jsonBody->get("system_prompt", "ASSISTANT's RULE: ").asString();
    this->pre_prompt = jsonBody->get("pre_prompt", "").asString();
    this->repeat_last_n = jsonBody->get("repeat_last_n", 32).asInt();

    if (!jsonBody->operator[]("llama_log_folder").isNull()) {
      log_enable();
      std::string llama_log_folder =
          jsonBody->operator[]("llama_log_folder").asString();
      log_set_target(llama_log_folder + "llama.log");
    }  // Set folder for llama log
  }
  if (params.model_alias == "unknown") {
    params.model_alias = params.model;
  }

  llama_backend_init();

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
    return false;  // Indicate failure
  }
  llama.initialize();

  if (queue != nullptr) {
    delete queue;
  }

  queue = new trantor::ConcurrentTaskQueue(llama.params.n_parallel, "llamaCPP");

  llama.model_loaded_external = true;

  LOG_INFO << "Started background task here!";
  backgroundThread = std::thread(&llamaCPP::BackgroundTask, this);

  // For model like nomic-embed-text-v1.5.f16.gguf, etc, we don't need to warm up model.
  // So we use this variable to differentiate with other models
  if (llama.model_type == ModelType::LLM) {
    WarmupModel();
  }
  return true;
}

void llamaCPP::BackgroundTask() {
  while (llama.model_loaded_external) {
    // model_loaded =
    llama.update_slots();
  }
  LOG_INFO << "Background task stopped! ";
  llama.kv_cache_clear();
  LOG_INFO << "KV cache cleared!";
  return;
}

void llamaCPP::StopBackgroundTask() {
  if (llama.model_loaded_external) {
    llama.model_loaded_external = false;
    llama.condition_tasks.notify_one();
    LOG_INFO << "Stopping background task! ";
    if (backgroundThread.joinable()) {
      backgroundThread.join();
    }
    LOG_INFO << "Background task stopped! ";
  }
}
