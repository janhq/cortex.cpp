#include "server.h"

#include <chrono>
#include <fstream>
#include <iostream>

#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/cpuid/cpu_info.h"
#include "utils/logging_utils.h"

using namespace inferences;
using json = nlohmann::json;
namespace inferences {
namespace {
constexpr static auto kLlamaEngine = "cortex.llamacpp";
constexpr static auto kPythonRuntimeEngine = "cortex.python";
constexpr static auto kOnnxEngine = "cortex.onnx";
constexpr static auto kTensorrtLlmEngine = "cortex.tensorrt-llm";
}  // namespace

server::server() {

  // Some default values for now below
  // log_disable();  // Disable the log to file feature, reduce bloat for
  // target
  // system ()
};

server::~server() {}

void server::ChatCompletion(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!HasFieldInReq(req, callback, "engine")) {
    return;
  }

  auto engine_type =
      (*(req->getJsonObject())).get("engine", cur_engine_type_).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start chat completion";
  auto json_body = req->getJsonObject();
  bool is_stream = (*json_body).get("stream", false).asBool();
  auto q = std::make_shared<SyncQueue>();
  std::get<EngineI*>(engines_[engine_type].engine)
      ->HandleChatCompletion(json_body,
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
  auto engine_type =
      (*(req->getJsonObject())).get("engine", kLlamaEngine).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start embedding";
  SyncQueue q;
  std::get<EngineI*>(engines_[engine_type].engine)
      ->HandleEmbedding(req->getJsonObject(),
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
  if (!HasFieldInReq(req, callback, "engine")) {
    return;
  }

  auto engine_type =
      (*(req->getJsonObject())).get("engine", cur_engine_type_).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }
  LOG_TRACE << "Start unload model";
  std::get<EngineI*>(engines_[engine_type].engine)
      ->UnloadModel(
          req->getJsonObject(),
          [cb = std::move(callback)](Json::Value status, Json::Value res) {
            auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
            resp->setStatusCode(static_cast<drogon::HttpStatusCode>(
                status["status_code"].asInt()));
            cb(resp);
          });
  LOG_TRACE << "Done unload model";
}

void server::ModelStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!HasFieldInReq(req, callback, "engine")) {
    return;
  }

  auto engine_type =
      (*(req->getJsonObject())).get("engine", cur_engine_type_).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start to get model status";
  std::get<EngineI*>(engines_[engine_type].engine)
      ->GetModelStatus(
          req->getJsonObject(),
          [cb = std::move(callback)](Json::Value status, Json::Value res) {
            auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
            resp->setStatusCode(static_cast<drogon::HttpStatusCode>(
                status["status_code"].asInt()));
            cb(resp);
          });
  LOG_TRACE << "Done get model status";
}

void server::GetModels(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  if (engines_.empty()) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start to get models";
  Json::Value resp_data(Json::arrayValue);
  for (auto const& [k, v] : engines_) {
    auto e = std::get<EngineI*>(v.engine);
    if (e->IsSupported("GetModels")) {
      e->GetModels(req->getJsonObject(),
                   [&resp_data](Json::Value status, Json::Value res) {
                     for (auto r : res["data"]) {
                       resp_data.append(r);
                     }
                   });
    }
  }
  Json::Value root;
  root["data"] = resp_data;
  root["object"] = "list";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(root);
  resp->setStatusCode(drogon::HttpStatusCode::k200OK);
  callback(resp);

  LOG_TRACE << "Done get models";
}

void server::GetEngines(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  Json::Value res;
  Json::Value engine_array(Json::arrayValue);
  for (const auto& [s, _] : engines_) {
    Json::Value val;
    val["id"] = s;
    val["object"] = "engine";
    engine_array.append(val);
  }

  res["object"] = "list";
  res["data"] = engine_array;

  auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
  callback(resp);
}

void server::FineTuning(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto engine_type =
      (*(req->getJsonObject())).get("engine", kPythonRuntimeEngine).asString();

  if (engines_.find(engine_type) == engines_.end()) {
    try {
      std::string abs_path =
          (getenv("ENGINE_PATH") ? getenv("ENGINE_PATH")
                                 : cortex_utils::GetCurrentPath()) +
          cortex_utils::kPythonRuntimeLibPath;
      engines_[engine_type].dl =
          std::make_unique<cortex_cpp::dylib>(abs_path, "engine");
    } catch (const cortex_cpp::dylib::load_error& e) {

      LOG_ERROR << "Could not load engine: " << e.what();
      engines_.erase(engine_type);

      Json::Value res;
      res["message"] = "Could not load engine " + engine_type;
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
      resp->setStatusCode(k500InternalServerError);
      callback(resp);
      return;
    }

    auto func = engines_[engine_type].dl->get_function<CortexPythonEngineI*()>(
        "get_engine");
    engines_[engine_type].engine = func();
    LOG_INFO << "Loaded engine: " << engine_type;
  }

  LOG_TRACE << "Start to fine-tuning";
  auto& en = std::get<CortexPythonEngineI*>(engines_[engine_type].engine);
  if (en->IsSupported("HandlePythonFileExecutionRequest")) {
    en->HandlePythonFileExecutionRequest(
        req->getJsonObject(),
        [cb = std::move(callback)](Json::Value status, Json::Value res) {
          auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
          resp->setStatusCode(static_cast<drogon::HttpStatusCode>(
              status["status_code"].asInt()));
          cb(resp);
        });
  } else {
    Json::Value res;
    res["message"] = "Method is not supported yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
    LOG_WARN << "Method is not supported yet";
  }
  LOG_TRACE << "Done fine-tuning";
}

void server::LoadModel(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  auto engine_type =
      (*(req->getJsonObject())).get("engine", kLlamaEngine).asString();

  // We have not loaded engine yet, should load it before using it
  if (engines_.find(engine_type) == engines_.end()) {
    auto get_engine_path = [](std::string_view e) {
      if (e == kLlamaEngine) {
        return cortex_utils::kLlamaLibPath;
      } else if (e == kOnnxEngine) {
        return cortex_utils::kOnnxLibPath;
      } else if (e == kTensorrtLlmEngine) {
        return cortex_utils::kTensorrtLlmPath;
      }
      return cortex_utils::kLlamaLibPath;
    };

    try {
      if (engine_type == kLlamaEngine) {
        cortex::cpuid::CpuInfo cpu_info;
        LOG_INFO << "CPU instruction set: " << cpu_info.to_string();
      }

      std::string abs_path =
          (getenv("ENGINE_PATH") ? getenv("ENGINE_PATH")
                                 : cortex_utils::GetCurrentPath()) +
          get_engine_path(engine_type);
      engines_[engine_type].dl =
          std::make_unique<cortex_cpp::dylib>(abs_path, "engine");

    } catch (const cortex_cpp::dylib::load_error& e) {
      LOG_ERROR << "Could not load engine: " << e.what();
      engines_.erase(engine_type);

      Json::Value res;
      res["message"] = "Could not load engine " + engine_type;
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
      resp->setStatusCode(k500InternalServerError);
      callback(resp);
      return;
    }
    cur_engine_type_ = engine_type;

    auto func =
        engines_[engine_type].dl->get_function<EngineI*()>("get_engine");
    engines_[engine_type].engine = func();
    LOG_INFO << "Loaded engine: " << engine_type;
  }

  LOG_TRACE << "Load model";
  auto& en = std::get<EngineI*>(engines_[engine_type].engine);
  en->LoadModel(req->getJsonObject(), [cb = std::move(callback)](
                                          Json::Value status, Json::Value res) {
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(
        static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
    cb(resp);
  });
  LOG_TRACE << "Done load model";
}

void server::UnloadEngine(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!HasFieldInReq(req, callback, "engine")) {
    return;
  }

  auto engine_type =
      (*(req->getJsonObject())).get("engine", cur_engine_type_).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  EngineI* e = std::get<EngineI*>(engines_[engine_type].engine);
  delete e;
  engines_.erase(engine_type);
  LOG_INFO << "Unloaded engine " + engine_type;
  Json::Value res;
  res["message"] = "Unloaded engine " + engine_type;
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
  resp->setStatusCode(k200OK);
  callback(resp);
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

  auto resp = cortex_utils::CreateCortexStreamResponse(chunked_content_provider,
                                                       "chat_completions.txt");
  cb(resp);
}

void server::ProcessNonStreamRes(std::function<void(const HttpResponsePtr&)> cb,
                                 SyncQueue& q) {
  auto [status, res] = q.wait_and_pop();
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
  resp->setStatusCode(
      static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
  cb(resp);
}

bool server::IsEngineLoaded(const std::string& e) {
  return engines_.find(e) != engines_.end();
}

bool server::HasFieldInReq(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>& callback,
    const std::string& field) {
  if (auto o = req->getJsonObject(); !o || (*o)[field].isNull()) {
    Json::Value res;
    res["message"] = "No " + field + " field in request body";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "No " << field << " field in request body";
    return false;
  }
  return true;
}

}  // namespace inferences
