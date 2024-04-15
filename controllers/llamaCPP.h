#include <string>
#if defined(_WIN32)
#define NOMINMAX
#endif
#pragma once

#include <drogon/HttpController.h>

#ifndef NDEBUG
// crash the server in debug mode, otherwise send an http 500 error
#define CPPHTTPLIB_NO_EXCEPTIONS 1
#endif

#include <trantor/utils/ConcurrentTaskQueue.h>
#include <cstddef>
#include <string>
#include <thread>

#include "common/base.h"
#include "context/llama_server_context.h"
#include "stb_image.h"
#include "utils/json.hpp"

#include "models/chat_completion_request.h"

#ifndef SERVER_VERBOSE
#define SERVER_VERBOSE 1
#endif

using json = nlohmann::json;

using namespace drogon;

namespace inferences {

class llamaCPP : public drogon::HttpController<llamaCPP>,
                 public BaseModel,
                 public BaseChatCompletion,
                 public BaseEmbedding {
 public:
  llamaCPP();
  ~llamaCPP();
  METHOD_LIST_BEGIN
  // list path definitions here;
  METHOD_ADD(llamaCPP::ChatCompletion, "chat_completion", Post);
  METHOD_ADD(llamaCPP::Embedding, "embedding", Post);
  METHOD_ADD(llamaCPP::LoadModel, "loadmodel", Post);
  METHOD_ADD(llamaCPP::UnloadModel, "unloadmodel", Get);
  METHOD_ADD(llamaCPP::ModelStatus, "modelstatus", Get);

  // Openai compatible path
  ADD_METHOD_TO(llamaCPP::ChatCompletion, "/v1/chat/completions", Post);
  // ADD_METHOD_TO(llamaCPP::handlePrelight, "/v1/chat/completions", Options);
  // NOTE: prelight will be added back when browser support is properly planned

  ADD_METHOD_TO(llamaCPP::Embedding, "/v1/embeddings", Post);
  // ADD_METHOD_TO(llamaCPP::handlePrelight, "/v1/embeddings", Options);

  // PATH_ADD("/llama/chat_completion", Post);
  METHOD_LIST_END
  void ChatCompletion(
      inferences::ChatCompletionRequest &&completion,
      std::function<void(const HttpResponsePtr&)>&& callback) override;
  void Embedding(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;
  void LoadModel(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback) override;
  void UnloadModel(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;
  void ModelStatus(
      const HttpRequestPtr& req,
      std::function<void(const HttpResponsePtr&)>&& callback) override;

 private:
  llama_server_context llama;
  // std::atomic<bool> model_loaded = false;
  size_t sent_count = 0;
  size_t sent_token_probs_index = 0;
  std::thread backgroundThread;
  std::string user_prompt;
  std::string ai_prompt;
  std::string system_prompt;
  std::string pre_prompt;
  int repeat_last_n;
  bool caching_enabled;
  std::atomic<int> no_of_requests = 0;
  std::atomic<int> no_of_chats = 0;
  int clean_cache_threshold;
  std::string grammar_file_content;

  /**
   * Queue to handle the inference tasks
   */
  trantor::ConcurrentTaskQueue* queue;

  bool LoadModelImpl(std::shared_ptr<Json::Value> jsonBody);
  void InferenceImpl(inferences::ChatCompletionRequest&& completion,
                     std::function<void(const HttpResponsePtr&)>&& callback);
  void EmbeddingImpl(std::shared_ptr<Json::Value> jsonBody,
                     std::function<void(const HttpResponsePtr&)>&& callback);
  bool CheckModelLoaded(const std::function<void(const HttpResponsePtr&)>& callback);
  void WarmupModel();
  void BackgroundTask();
  void StopBackgroundTask();
};
};  // namespace inferences
