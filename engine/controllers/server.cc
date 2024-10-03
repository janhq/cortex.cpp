#include "server.h"

#include "trantor/utils/Logger.h"
#include "utils/cortex_utils.h"
#include "utils/cpuid/cpu_info.h"
#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"

using namespace inferences;
using json = nlohmann::json;
namespace inferences {
namespace {
// Need to change this after we rename repositories
std::string NormalizeEngine(const std::string& engine) {
  if (engine == kLlamaEngine) {
    return kLlamaRepo;
  } else if (engine == kOnnxEngine) {
    return kOnnxRepo;
  } else if (engine == kTrtLlmEngine) {
    return kTrtLlmRepo;
  }
  return engine;
};
}  // namespace
server::server() {
#if defined(_WIN32)
  SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#endif
};

server::~server() {}

void server::ChatCompletion(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  std::string engine_type;
  if (!HasFieldInReq(req, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type =
        (*(req->getJsonObject())).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);

  if (!IsEngineLoaded(ne)) {
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
  std::get<EngineI*>(engines_[ne].engine)
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
      (*(req->getJsonObject())).get("engine", kLlamaRepo).asString();
  auto ne = NormalizeEngine(engine_type);
  if (!IsEngineLoaded(ne)) {
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
  std::get<EngineI*>(engines_[ne].engine)
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
  std::string engine_type;
  if (!HasFieldInReq(req, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type =
        (*(req->getJsonObject())).get("engine", kLlamaRepo).asString();
  }
  auto ne = NormalizeEngine(engine_type);

  if (!IsEngineLoaded(ne)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }
  LOG_TRACE << "Start unload model";
  std::get<EngineI*>(engines_[ne].engine)
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
  std::string engine_type;
  if (!HasFieldInReq(req, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type =
        (*(req->getJsonObject())).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);

  if (!IsEngineLoaded(ne)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start to get model status";
  std::get<EngineI*>(engines_[ne].engine)
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
      (*(req->getJsonObject())).get("engine", kPythonRuntimeRepo).asString();

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
      (*(req->getJsonObject())).get("engine", kLlamaRepo).asString();

  auto ne = NormalizeEngine(engine_type);

  // We have not loaded engine yet, should load it before using it
  if (engines_.find(ne) == engines_.end()) {
    auto get_engine_path = [](std::string_view e) {
      if (e == kLlamaRepo) {
        return cortex_utils::kLlamaLibPath;
      } else if (e == kOnnxRepo) {
        return cortex_utils::kOnnxLibPath;
      } else if (e == kTrtLlmRepo) {
        return cortex_utils::kTensorrtLlmPath;
      }
      return cortex_utils::kLlamaLibPath;
    };

    try {
      if (ne == kLlamaRepo) {
        cortex::cpuid::CpuInfo cpu_info;
        LOG_INFO << "CPU instruction set: " << cpu_info.to_string();
      }

      std::string abs_path =
          (getenv("ENGINE_PATH")
               ? getenv("ENGINE_PATH")
               : file_manager_utils::GetCortexDataPath().string()) +
          get_engine_path(ne);
#if defined(_WIN32)
      // TODO(?) If we only allow to load an engine at a time, the logic is simpler.
      // We would like to support running multiple engines at the same time. Therefore,
      // the adding/removing dll directory logic is quite complicated:
      // 1. If llamacpp is loaded and new requested engine is tensorrt-llm:
      // Unload the llamacpp dll directory then load the tensorrt-llm
      // 2. If tensorrt-llm is loaded and new requested engine is llamacpp:
      // Do nothing, llamacpp can re-use tensorrt-llm dependencies (need to be tested careful)
      // 3. Add dll directory if met other conditions

      auto add_dll = [this](const std::string& e_type, const std::string& p) {
        auto ws = std::wstring(p.begin(), p.end());
        if (auto cookie = AddDllDirectory(ws.c_str()); cookie != 0) {
          LOG_INFO << "Added dll directory: " << p;
          engines_[e_type].cookie = cookie;
        } else {
          LOG_WARN << "Could not add dll directory: " << p;
        }
      };

      if (IsEngineLoaded(kLlamaRepo) && ne == kTrtLlmRepo) {
        // Remove llamacpp dll directory
        if (!RemoveDllDirectory(engines_[kLlamaRepo].cookie)) {
          LOG_INFO << "Could not remove dll directory: " << kLlamaRepo;
        } else {
          LOG_WARN << "Removed dll directory: " << kLlamaRepo;
        }

        add_dll(ne, abs_path);
      } else if (IsEngineLoaded(kTrtLlmRepo) && ne == kLlamaRepo) {
        // Do nothing
      } else {
        add_dll(ne, abs_path);
      }
#endif
      engines_[ne].dl = std::make_unique<cortex_cpp::dylib>(abs_path, "engine");

    } catch (const cortex_cpp::dylib::load_error& e) {
      LOG_ERROR << "Could not load engine: " << e.what();
      engines_.erase(ne);

      Json::Value res;
      res["message"] = "Could not load engine " + engine_type;
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
      resp->setStatusCode(k500InternalServerError);
      callback(resp);
      return;
    }
    cur_engine_type_ = ne;

    auto func = engines_[ne].dl->get_function<EngineI*()>("get_engine");
    engines_[ne].engine = func();

    auto& en = std::get<EngineI*>(engines_[ne].engine);
    if (ne == kLlamaRepo) {  //fix for llamacpp engine first
      auto config = file_manager_utils::GetCortexConfig();
      if (en->IsSupported("SetFileLogger")) {
        en->SetFileLogger(config.maxLogLines,
                          (std::filesystem::path(config.logFolderPath) /
                           std::filesystem::path(config.logLlamaCppPath))
                              .string());
      } else {
        LOG_WARN << "Method SetFileLogger is not supported yet";
      }
    }
    LOG_INFO << "Loaded engine: " << engine_type;
  }

  LOG_TRACE << "Load model";
  auto& en = std::get<EngineI*>(engines_[ne].engine);
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
  std::string engine_type;
  if (!HasFieldInReq(req, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type =
        (*(req->getJsonObject())).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);

  if (!IsEngineLoaded(ne)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  EngineI* e = std::get<EngineI*>(engines_[ne].engine);
  delete e;
#if defined(_WIN32)
  if (!RemoveDllDirectory(engines_[ne].cookie)) {
    LOG_WARN << "Could not remove dll directory: " << engine_type;
  } else {
    LOG_INFO << "Removed dll directory: " << engine_type;
  }
#endif
  engines_.erase(ne);
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

bool server::HasFieldInReq(const HttpRequestPtr& req,
                           const std::string& field) {
  if (auto o = req->getJsonObject(); !o || (*o)[field].isNull()) {
    return false;
  }
  return true;
}

}  // namespace inferences
