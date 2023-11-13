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
using json = nlohmann::json;

// To store state of each inference request
struct State {
  bool isStopped = false;
  int task_id;
  llamaCPP *instance;

  State(int tid, llamaCPP *inst) : task_id(tid), instance(inst) {}
};

std::shared_ptr<State> createState(int task_id, llamaCPP *instance) {
  return std::make_shared<State>(task_id, instance);
}

// --------------------------------------------

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
  //  json pseudo;
  //
  //  pseudo["prompt"] = "Hello";
  //  pseudo["n_predict"] = 10;
  //  const int task_id = llama.request_completion(pseudo, false);
  //  std::string completion_text;
  //  task_result result = llama.next_result(task_id);
  //  if (!result.error && result.stop) {
  //    LOG_INFO << result.result_json.dump(-1, ' ', false,
  //                                        json::error_handler_t::replace);
  //  }
  //  return;
}

void llamaCPP::chatCompletion(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

  const auto &jsonBody = req->getJsonObject();
  std::string formatted_output = pre_prompt;

  json data;
  json stopWords;
  // To set default value
  data["stream"] = true;

  if (jsonBody) {
    data["n_predict"] = (*jsonBody).get("max_tokens", 500).asInt();
    data["top_p"] = (*jsonBody).get("top_p", 0.95).asFloat();
    data["temperature"] = (*jsonBody).get("temperature", 0.8).asFloat();
    data["frequency_penalty"] =
        (*jsonBody).get("frequency_penalty", 0).asFloat();
    data["presence_penalty"] = (*jsonBody).get("presence_penalty", 0).asFloat();

    const Json::Value &messages = (*jsonBody)["messages"];
    for (const auto &message : messages) {
      std::string input_role = message["role"].asString();
      std::string role;
      if (input_role == "user") {
        role = user_prompt;
      } else if (input_role == "assistant") {
        role = ai_prompt;
      } else if (input_role == "system") {
        role = system_prompt;
      } else {
        role = input_role;
      }
      std::string content = message["content"].asString();
      formatted_output += role + content + "\n";
    }
    formatted_output += ai_prompt;

    data["prompt"] = formatted_output;
    for (const auto &stop_word : (*jsonBody)["stop"]) {
      stopWords.push_back(stop_word.asString());
    }
    // specify default stop words
    stopWords.push_back(user_prompt);
    data["stop"] = stopWords;
  }

  const int task_id = llama.request_completion(data, false, false);
  LOG_INFO << "Resolved request for task_id:" << task_id;

  auto state = createState(task_id, this);

  auto chunked_content_provider =
      [state](char *pBuffer, std::size_t nBuffSize) -> std::size_t {
    if (!pBuffer) {
      LOG_INFO << "Connection closed or buffer is null. Reset context";
      state->instance->llama.request_cancel(state->task_id);
      return 0;
    }
    if (state->isStopped) {
      return 0;
    }

    task_result result = state->instance->llama.next_result(state->task_id);
    if (!result.error) {
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
            create_return_json(nitro_utils::generate_random_string(20), "_", "",
                               "stop") +
            "\n\n" + "data: [DONE]" + "\n\n";

        LOG_VERBOSE("data stream", {{"to_send", str}});
        std::size_t nRead = std::min(str.size(), nBuffSize);
        memcpy(pBuffer, str.data(), nRead);
        LOG_INFO << "reached result stop";
        state->isStopped = true;
        state->instance->llama.request_cancel(state->task_id);
        return nRead;
      }
      return nRead;
    } else {
      return 0;
    }
    return 0;
  };
  auto resp = nitro_utils::nitroStreamResponse(chunked_content_provider,
                                               "chat_completions.txt");
  callback(resp);

  return;
}

void llamaCPP::embedding(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  const auto &jsonBody = req->getJsonObject();

  json prompt;
  if (jsonBody->isMember("content") != 0) {
    prompt = (*jsonBody)["content"].asString();
  } else {
    prompt = "";
  }
  const int task_id = llama.request_completion(
      {{"prompt", prompt}, {"n_predict", 0}}, false, true);
  task_result result = llama.next_result(task_id);
  std::string embeddingResp = result.result_json.dump();
  auto resp = nitro_utils::nitroHttpResponse();
  resp->setBody(embeddingResp);
  resp->setContentTypeString("application/json");
  callback(resp);
  return;
}

void llamaCPP::unloadModel(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  Json::Value jsonResp;
  jsonResp["message"] = "No model loaded";
  if (model_loaded) {
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

void llamaCPP::loadModel(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {

  const auto &jsonBody = req->getJsonObject();

  gpt_params params;

  params.cont_batching = false;
  // By default will setting based on number of handlers
  int drogon_thread = drogon::app().getThreadNum();
  LOG_INFO << "Drogon thread is:" << drogon_thread;
  if (jsonBody) {
    params.model = (*jsonBody)["llama_model_path"].asString();
    params.n_gpu_layers = (*jsonBody).get("ngl", 100).asInt();
    params.n_ctx = (*jsonBody).get("ctx_len", 2048).asInt();
    params.embedding = (*jsonBody).get("embedding", true).asBool();
    // Check if n_parallel exists in jsonBody, if not, set to drogon_thread

    params.n_parallel = (*jsonBody).get("n_parallel", drogon_thread).asInt();

    params.cont_batching = (*jsonBody)["cont_batching"].asBool();

    this->user_prompt = (*jsonBody).get("user_prompt", "USER: ").asString();
    this->ai_prompt = (*jsonBody).get("ai_prompt", "ASSISTANT: ").asString();
    this->system_prompt =
        (*jsonBody).get("system_prompt", "ASSISTANT's RULE: ").asString();
    this->pre_prompt =
        (*jsonBody)
            .get("pre_prompt",
                 "A chat between a curious user and an artificial intelligence "
                 "assistant. The assistant follows the given rules no matter "
                 "what.\\n")
            .asString();
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
    LOG_ERROR << "Error loading the model will exit the program";
    Json::Value jsonResp;
    jsonResp["message"] = "Model loaded failed";
    auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
    resp->setStatusCode(drogon::k500InternalServerError);
    callback(resp);
  }
  llama.initialize();

  Json::Value jsonResp;
  jsonResp["message"] = "Model loaded successfully";
  model_loaded = true;
  auto resp = nitro_utils::nitroHttpJsonResponse(jsonResp);
  // warmupModel();

  LOG_INFO << "Started background task here!";
  backgroundThread = std::thread(&llamaCPP::backgroundTask, this);
  callback(resp);
}

void llamaCPP::backgroundTask() {
  while (model_loaded) {
    // model_loaded =
    llama.update_slots();
  }
  LOG_INFO << "Background task stopped!";
  return;
}

void llamaCPP::stopBackgroundTask() {
  if (model_loaded) {
    model_loaded = false;
    LOG_INFO << "changed to false";
    if (backgroundThread.joinable()) {
      backgroundThread.join();
    }
  }
}
