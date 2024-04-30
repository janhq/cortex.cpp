#include "llamaCPP.h"

#include <chrono>
#include <fstream>
#include <iostream>

#include "trantor/utils/Logger.h"
#include "utils/logging_utils.h"
#include "utils/nitro_utils.h"

using namespace inferences;
using json = nlohmann::json;
namespace inferences {
llamaCPP::llamaCPP() {
  dylib_ = std::make_unique<dylib>("./engines/cortex.llamacpp", "engine");
  auto func = dylib_->get_function<EngineI*()>("get_engine");
  engine_ = func();
  // Some default values for now below
  // log_disable();  // Disable the log to file feature, reduce bloat for
  // target
  // system ()
};

llamaCPP::~llamaCPP() {}

void llamaCPP::ChatCompletion(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  LOG_TRACE << "Start chat completion";
  auto json_body = req->getJsonObject();
  bool is_stream = (*json_body).get("stream", false).asBool();
  auto q = std::make_shared<SyncQueue>();
  engine_->HandleChatCompletion(json_body,
                                [q](Json::Value status, Json::Value res) {
                                  q->push(std::make_pair(status, res));
                                });
  LOG_TRACE << "Wait to chat completion responses";
  if (is_stream) {
    ProcessStreamRes(std::move(callback), q);
  } else {
    ProcessNonStreamRes(std::move(callback), *q);
  }
  LOG_TRACE << "Done chat completion";
}

void llamaCPP::Embedding(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  LOG_TRACE << "Start embedding";
  SyncQueue q;
  engine_->HandleEmbedding(req->getJsonObject(),
                           [&q](Json::Value status, Json::Value res) {
                             q.push(std::make_pair(status, res));
                           });
  LOG_TRACE << "Wait to embedding";
  ProcessNonStreamRes(std::move(callback), q);
  LOG_TRACE << "Done embedding";
}

void llamaCPP::UnloadModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  LOG_TRACE << "Start unload model";
  engine_->UnloadModel(
      req->getJsonObject(),
      [cb = std::move(callback)](Json::Value status, Json::Value res) {
        auto resp = nitro_utils::nitroHttpJsonResponse(res);
        resp->setStatusCode(
            static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
        cb(resp);
      });
  LOG_TRACE << "Done unload model";
}

void llamaCPP::ModelStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  LOG_TRACE << "Start to get model status";
  engine_->GetModelStatus(
      req->getJsonObject(),
      [cb = std::move(callback)](Json::Value status, Json::Value res) {
        auto resp = nitro_utils::nitroHttpJsonResponse(res);
        resp->setStatusCode(
            static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
        cb(resp);
      });
  LOG_TRACE << "Done get model status";
}

void llamaCPP::LoadModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  LOG_TRACE << "Load model";
  engine_->LoadModel(
      req->getJsonObject(),
      [cb = std::move(callback)](Json::Value status, Json::Value res) {
        auto resp = nitro_utils::nitroHttpJsonResponse(res);
        resp->setStatusCode(
            static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
        cb(resp);
      });
  LOG_TRACE << "Done load model";
}

void llamaCPP::ProcessStreamRes(std::function<void(const HttpResponsePtr&)> cb,
                                std::shared_ptr<SyncQueue> q) {
  auto err_or_done = std::make_shared<std::atomic_bool>(false);
  auto chunked_content_provider = [q, err_or_done](
                                      char* buf,
                                      std::size_t buf_size) -> std::size_t {
    if (buf == nullptr) {
      LOG_TRACE << "Buf is null";
      return 0;
    }

    if (*err_or_done) {
      LOG_TRACE << "Done";
      return 0;
    }

    auto [status, res] = q->wait_and_pop();

    if (status["has_error"].asBool() || status["is_done"].asBool()) {
      *err_or_done = true;
    }

    auto str = res["data"].asString();
    LOG_TRACE << "data: " << str;
    std::size_t n = std::min(str.size(), buf_size);
    memcpy(buf, str.data(), n);

    return n;
  };

  auto resp = nitro_utils::nitroStreamResponse(chunked_content_provider,
                                               "chat_completions.txt");
  cb(resp);
}

void llamaCPP::ProcessNonStreamRes(
    std::function<void(const HttpResponsePtr&)> cb, SyncQueue& q) {
  auto [status, res] = q.wait_and_pop();
  auto resp = nitro_utils::nitroHttpJsonResponse(res);
  resp->setStatusCode(
      static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
  cb(resp);
}
}  // namespace inferences