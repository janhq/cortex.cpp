#include "server.h"

#include <chrono>
#include <fstream>
#include <iostream>

#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

using namespace inferences;
using json = nlohmann::json;
namespace inferences {
namespace {
constexpr static auto kLlamaEngine = "cortex.llamacpp";
constexpr static auto kLlamaLibPath = "/engines/cortex.llamacpp";
}  // namespace

server::server()
    : engine_{nullptr} {

          // Some default values for now below
          // log_disable();  // Disable the log to file feature, reduce bloat for
          // target
          // system ()
      };

server::~server() {}

void server::ChatCompletion(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!IsEngineLoaded()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

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

void server::Embedding(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!IsEngineLoaded()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

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

void server::UnloadModel(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!IsEngineLoaded()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }
  LOG_TRACE << "Start unload model";
  engine_->UnloadModel(
      req->getJsonObject(),
      [cb = std::move(callback)](Json::Value status, Json::Value res) {
        auto resp = cortex_utils::nitroHttpJsonResponse(res);
        resp->setStatusCode(
            static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
        cb(resp);
      });
  LOG_TRACE << "Done unload model";
}

void server::ModelStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!IsEngineLoaded()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start to get model status";
  engine_->GetModelStatus(
      req->getJsonObject(),
      [cb = std::move(callback)](Json::Value status, Json::Value res) {
        auto resp = cortex_utils::nitroHttpJsonResponse(res);
        resp->setStatusCode(
            static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
        cb(resp);
      });
  LOG_TRACE << "Done get model status";
}

void server::LoadModel(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  auto engine_type =
      (*(req->getJsonObject())).get("engine", kLlamaEngine).asString();
  if (!dylib_ || engine_type != cur_engine_name_) {
    cur_engine_name_ = engine_type;
    // TODO: change this when we get more engines
    auto get_engine_path = [](std::string_view e) {
      if (e == kLlamaEngine) {
        return kLlamaLibPath;
      }
      return kLlamaLibPath;
    };

    try {
      std::string abs_path = cortex_utils::GetCurrentPath() +
                             get_engine_path(cur_engine_name_);
      dylib_ =
          std::make_unique<cortex_cpp::dylib>(abs_path, "engine");
    } catch (const cortex_cpp::dylib::load_error& e) {
      LOG_ERROR << "Could not load engine: " << e.what();
      dylib_.reset();
      engine_ = nullptr;
    }

    if (!dylib_) {
      Json::Value res;
      res["message"] = "Could not load engine " + cur_engine_name_;
      auto resp = cortex_utils::nitroHttpJsonResponse(res);
      resp->setStatusCode(k500InternalServerError);
      callback(resp);
      return;
    }
    auto func = dylib_->get_function<EngineI*()>("get_engine");
    engine_ = func();
    LOG_INFO << "Loaded engine: " << cur_engine_name_;
  }

  LOG_TRACE << "Load model";
  engine_->LoadModel(
      req->getJsonObject(),
      [cb = std::move(callback)](Json::Value status, Json::Value res) {
        auto resp = cortex_utils::nitroHttpJsonResponse(res);
        resp->setStatusCode(
            static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
        cb(resp);
      });
  LOG_TRACE << "Done load model";
}

void server::ProcessStreamRes(std::function<void(const HttpResponsePtr&)> cb,
                              std::shared_ptr<SyncQueue> q) {
  auto err_or_done = std::make_shared<std::atomic_bool>(false);
  auto chunked_content_provider =
      [q, err_or_done](char* buf, std::size_t buf_size) -> std::size_t {
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

  auto resp = cortex_utils::nitroStreamResponse(chunked_content_provider,
                                                "chat_completions.txt");
  cb(resp);
}

void server::ProcessNonStreamRes(std::function<void(const HttpResponsePtr&)> cb,
                                 SyncQueue& q) {
  auto [status, res] = q.wait_and_pop();
  auto resp = cortex_utils::nitroHttpJsonResponse(res);
  resp->setStatusCode(
      static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
  cb(resp);
}

bool server::IsEngineLoaded() {
  return !!engine_;
}

}  // namespace inferences