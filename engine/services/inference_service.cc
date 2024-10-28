#include "inference_service.h"
#include "utils/cpuid/cpu_info.h"
#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"
#include "utils/function_calling/common.h"

namespace services {

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

constexpr const int k200OK = 200;
constexpr const int k400BadRequest = 400;
constexpr const int k409Conflict = 409;
constexpr const int k500InternalServerError = 500;
}  // namespace

cpp::result<void, InferResult> InferenceService::HandleChatCompletion(
    std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }
  auto ne = NormalizeEngine(engine_type);
  if (!IsEngineLoaded(ne)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    Json::Value stt;
    stt["status_code"] = k409Conflict;
    LOG_WARN << "Engine is not loaded yet";
    return cpp::fail(std::make_pair(stt, res));
  }

  function_calling_utils::PreprocessRequest(json_body);
  Json::Value tool_choice = json_body->get("tool_choice", Json::Value::null);
  std::get<EngineI*>(engines_[ne].engine)
      ->HandleChatCompletion(
          json_body, [q, tool_choice](Json::Value status, Json::Value res) {
            if (!tool_choice.isNull()) {
              res["tool_choice"] = tool_choice;
            }
            q->push(std::make_pair(status, res));
          });
  return {};
}

cpp::result<void, InferResult> InferenceService::HandleEmbedding(
    std::shared_ptr<SyncQueue> q, std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);
  if (!IsEngineLoaded(ne)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    Json::Value stt;
    stt["status_code"] = k409Conflict;
    LOG_WARN << "Engine is not loaded yet";
    return cpp::fail(std::make_pair(stt, res));
  }
  std::get<EngineI*>(engines_["llama-cpp"].engine)
      ->HandleEmbedding(json_body, [q](Json::Value status, Json::Value res) {
        q->push(std::make_pair(status, res));
      });
  return {};
}

InferResult InferenceService::LoadModel(
    std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);
  Json::Value r;
  Json::Value stt;
  // We have not loaded engine yet, should load it before using it
  if (engines_.find(ne) == engines_.end()) {
    auto get_engine_path = [](std::string_view e) {
      if (e == kLlamaRepo) {
        return kLlamaLibPath;
      } else if (e == kOnnxRepo) {
        return kOnnxLibPath;
      } else if (e == kTrtLlmRepo) {
        return kTensorrtLlmPath;
      }
      return kLlamaLibPath;
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
      LOG_INFO << "engine path: " << abs_path;
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

      if (bool should_use_dll_search_path = !(getenv("ENGINE_PATH"));
          should_use_dll_search_path) {
        if (IsEngineLoaded(kLlamaRepo) && ne == kTrtLlmRepo &&
            should_use_dll_search_path) {
          // Remove llamacpp dll directory
          if (!RemoveDllDirectory(engines_[kLlamaRepo].cookie)) {
            LOG_WARN << "Could not remove dll directory: " << kLlamaRepo;
          } else {
            LOG_INFO << "Removed dll directory: " << kLlamaRepo;
          }

          add_dll(ne, abs_path);
        } else if (IsEngineLoaded(kTrtLlmRepo) && ne == kLlamaRepo) {
          // Do nothing
        } else {
          add_dll(ne, abs_path);
        }
      }
#endif
      engines_[ne].dl = std::make_unique<cortex_cpp::dylib>(abs_path, "engine");

    } catch (const cortex_cpp::dylib::load_error& e) {
      LOG_ERROR << "Could not load engine: " << e.what();
      engines_.erase(ne);

      r["message"] = "Could not load engine " + ne + ": " + e.what();
      stt["status_code"] = k500InternalServerError;
      return std::make_pair(stt, r);
    }

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
    LOG_INFO << "Loaded engine: " << ne;
  }

  // LOG_TRACE << "Load model";
  auto& en = std::get<EngineI*>(engines_[ne].engine);
  en->LoadModel(json_body, [&stt, &r](Json::Value status, Json::Value res) {
    stt = status;
    r = res;
  });
  return std::make_pair(stt, r);
}

InferResult InferenceService::UnloadModel(
    std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);
  Json::Value r;
  Json::Value stt;
  if (!IsEngineLoaded(ne)) {
    r["message"] = "Engine is not loaded yet";
    stt["status_code"] = k409Conflict;
    LOG_WARN << "Engine is not loaded yet";
    return std::make_pair(stt, r);
  }
  LOG_TRACE << "Start unload model";
  std::get<EngineI*>(engines_[ne].engine)
      ->UnloadModel(json_body, [&r, &stt](Json::Value status, Json::Value res) {
        stt = status;
        r = res;
      });
  return std::make_pair(stt, r);
}

InferResult InferenceService::GetModelStatus(
    std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);
  Json::Value r;
  Json::Value stt;

  if (!IsEngineLoaded(ne)) {
    r["message"] = "Engine is not loaded yet";
    stt["status_code"] = k409Conflict;
    LOG_WARN << "Engine is not loaded yet";
    return std::make_pair(stt, r);
  }

  LOG_TRACE << "Start to get model status";
  std::get<EngineI*>(engines_[ne].engine)
      ->GetModelStatus(json_body,
                       [&stt, &r](Json::Value status, Json::Value res) {
                         stt = status;
                         r = res;
                       });
  return std::make_pair(stt, r);
}

InferResult InferenceService::GetModels(
    std::shared_ptr<Json::Value> json_body) {
  Json::Value r;
  Json::Value stt;
  if (engines_.empty()) {
    r["message"] = "Engine is not loaded yet";
    stt["status_code"] = k409Conflict;
    return std::make_pair(stt, r);
  }

  LOG_TRACE << "Start to get models";
  Json::Value resp_data(Json::arrayValue);
  for (auto const& [k, v] : engines_) {
    auto e = std::get<EngineI*>(v.engine);
    if (e->IsSupported("GetModels")) {
      e->GetModels(json_body,
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
  stt["status_code"] = k200OK;
  return std::make_pair(stt, root);
  // LOG_TRACE << "Done get models";
}

Json::Value InferenceService::GetEngines(
    std::shared_ptr<Json::Value> json_body) {
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
  return res;
}

InferResult InferenceService::FineTuning(
    std::shared_ptr<Json::Value> json_body) {
  std::string ne = kPythonRuntimeRepo;
  Json::Value r;
  Json::Value stt;

  if (engines_.find(ne) == engines_.end()) {
    try {
      std::string abs_path =
          (getenv("ENGINE_PATH")
               ? getenv("ENGINE_PATH")
               : file_manager_utils::GetCortexDataPath().string()) +
          kPythonRuntimeLibPath;
      engines_[ne].dl = std::make_unique<cortex_cpp::dylib>(abs_path, "engine");
    } catch (const cortex_cpp::dylib::load_error& e) {

      LOG_ERROR << "Could not load engine: " << e.what();
      engines_.erase(ne);

      Json::Value res;
      r["message"] = "Could not load engine " + ne;
      stt["status_code"] = k500InternalServerError;
      return std::make_pair(stt, r);
    }

    auto func =
        engines_[ne].dl->get_function<CortexPythonEngineI*()>("get_engine");
    engines_[ne].engine = func();
    LOG_INFO << "Loaded engine: " << ne;
  }

  LOG_TRACE << "Start to fine-tuning";
  auto& en = std::get<CortexPythonEngineI*>(engines_[ne].engine);
  if (en->IsSupported("HandlePythonFileExecutionRequest")) {
    en->HandlePythonFileExecutionRequest(
        json_body, [&r, &stt](Json::Value status, Json::Value res) {
          r = res;
          stt = status;
        });
  } else {
    LOG_WARN << "Method is not supported yet";
    r["message"] = "Method is not supported yet";
    stt["status_code"] = k500InternalServerError;
    return std::make_pair(stt, r);
  }
  LOG_TRACE << "Done fine-tuning";
  return std::make_pair(stt, r);
}

InferResult InferenceService::UnloadEngine(
    std::shared_ptr<Json::Value> json_body) {
  std::string engine_type;
  if (!HasFieldInReq(json_body, "engine")) {
    engine_type = kLlamaRepo;
  } else {
    engine_type = (*(json_body)).get("engine", kLlamaRepo).asString();
  }

  auto ne = NormalizeEngine(engine_type);
  Json::Value r;
  Json::Value stt;

  if (!IsEngineLoaded(ne)) {
    r["message"] = "Engine is not loaded yet";
    stt["status_code"] = k409Conflict;
    LOG_WARN << "Engine is not loaded yet";
    return std::make_pair(stt, r);
  }

  EngineI* e = std::get<EngineI*>(engines_[ne].engine);
  delete e;
#if defined(_WIN32)
  if (bool should_use_dll_search_path = !(getenv("ENGINE_PATH"));
      should_use_dll_search_path) {
    if (!RemoveDllDirectory(engines_[ne].cookie)) {
      LOG_WARN << "Could not remove dll directory: " << ne;
    } else {
      LOG_INFO << "Removed dll directory: " << ne;
    }
  }
#endif
  engines_.erase(ne);
  LOG_INFO << "Unloaded engine " + ne;
  r["message"] = "Unloaded engine " + ne;
  stt["status_code"] = k200OK;
  return std::make_pair(stt, r);
}

bool InferenceService::IsEngineLoaded(const std::string& e) {
  return engines_.find(e) != engines_.end();
}

bool InferenceService::HasFieldInReq(std::shared_ptr<Json::Value> json_body,
                                     const std::string& field) {
  if (!json_body || (*json_body)[field].isNull()) {
    return false;
  }
  return true;
}
}  // namespace services
