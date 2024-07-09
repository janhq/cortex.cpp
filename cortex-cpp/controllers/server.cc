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
constexpr static auto kAudioEngine = "cortex.audio";
}  // namespace

server::server(){

    // Some default values for now below
    // log_disable();  // Disable the log to file feature, reduce bloat for
    // target
    // system ()
};

server::~server() {}

void server::ChatCompletion(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto engine_type =
      (*(req->getJsonObject())).get("engine", cur_engine_type_).asString();
  if (!IsEngineLoaded(engine_type)) {
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
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
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
  auto engine_type =
      (*(req->getJsonObject())).get("engine", cur_engine_type_).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
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
            auto resp = cortex_utils::nitroHttpJsonResponse(res);
            resp->setStatusCode(static_cast<drogon::HttpStatusCode>(
                status["status_code"].asInt()));
            cb(resp);
          });
  LOG_TRACE << "Done unload model";
}

void server::ModelStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto engine_type =
      (*(req->getJsonObject())).get("engine", cur_engine_type_).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
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
            auto resp = cortex_utils::nitroHttpJsonResponse(res);
            resp->setStatusCode(static_cast<drogon::HttpStatusCode>(
                status["status_code"].asInt()));
            cb(resp);
          });
  LOG_TRACE << "Done get model status";
}

void server::GetModels(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!IsEngineLoaded(cur_engine_type_)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start to get models";
  std::vector<std::string> e_types = {cur_engine_type_};
  if (cur_engine_type_ == kLlamaEngine) {
    e_types.push_back(kAudioEngine);
  } else if (cur_engine_type_ == kAudioEngine) {
    e_types.push_back(kLlamaEngine);
  }
  for (auto const& et : e_types) {
    if (IsEngineLoaded(et)) {
      auto& en = std::get<EngineI*>(engines_[et].engine);
      if (en->IsSupported("GetModels")) {
        en->GetModels(
            req->getJsonObject(),
            [cb = std::move(callback)](Json::Value status, Json::Value res) {
              auto resp = cortex_utils::nitroHttpJsonResponse(res);
              resp->setStatusCode(static_cast<drogon::HttpStatusCode>(
                  status["status_code"].asInt()));
              cb(resp);
            });
      } else {
        Json::Value res;
        res["message"] = "Method is not supported yet";
        auto resp = cortex_utils::nitroHttpJsonResponse(res);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
        LOG_WARN << "Method is not supported yet";
      }
    }
  }

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

  auto resp = cortex_utils::nitroHttpJsonResponse(res);
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
          cortex_utils::GetCurrentPath() + cortex_utils::kPythonRuntimeLibPath;
      engines_[engine_type].dl =
          std::make_unique<cortex_cpp::dylib>(abs_path, "engine");
    } catch (const cortex_cpp::dylib::load_error& e) {

      LOG_ERROR << "Could not load engine: " << e.what();
      engines_.erase(engine_type);

      Json::Value res;
      res["message"] = "Could not load engine " + engine_type;
      auto resp = cortex_utils::nitroHttpJsonResponse(res);
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
          auto resp = cortex_utils::nitroHttpJsonResponse(res);
          resp->setStatusCode(static_cast<drogon::HttpStatusCode>(
              status["status_code"].asInt()));
          cb(resp);
        });
  } else {
    Json::Value res;
    res["message"] = "Method is not supported yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
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
    // We use single engine for llamacpp, onnx, tensorrt so unload all engines before load new engine
    // But it is tricky that we can use llamacpp with audio engine
    UnloadEngines(engine_type);
    auto get_engine_path = [](std::string_view e) {
      if (e == kLlamaEngine) {
        return cortex_utils::kLlamaLibPath;
      } else if (e == kOnnxEngine) {
        return cortex_utils::kOnnxLibPath;
      } else if (e == kTensorrtLlmEngine) {
        return cortex_utils::kTensorrtLlmPath;
      } else if (e == kAudioEngine) {
        return cortex_utils::kAudioLibPath;
      }
      return cortex_utils::kLlamaLibPath;
    };

    try {
      if (engine_type == kLlamaEngine || engine_type == kAudioEngine) {
        cortex::cpuid::CpuInfo cpu_info;
        LOG_INFO << "CPU instruction set: " << cpu_info.to_string();
      }

      std::string abs_path =
          cortex_utils::GetCurrentPath() + get_engine_path(engine_type);
      engines_[engine_type].dl =
          std::make_unique<cortex_cpp::dylib>(abs_path, "engine");

    } catch (const cortex_cpp::dylib::load_error& e) {
      LOG_ERROR << "Could not load engine: " << e.what();
      engines_.erase(engine_type);

      Json::Value res;
      res["message"] = "Could not load engine " + engine_type;
      auto resp = cortex_utils::nitroHttpJsonResponse(res);
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
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(
        static_cast<drogon::HttpStatusCode>(status["status_code"].asInt()));
    cb(resp);
  });
  LOG_TRACE << "Done load model";
}

void server::CreateTranscription(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  if (!IsEngineLoaded(kAudioEngine)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start transcript";
  SyncQueue q;
  auto& en = std::get<EngineI*>(engines_[kAudioEngine].engine);
  if (en->IsSupported("CreateTranscription")) {
    auto req_body = ToAudioReqBody(req);
    if (req_body == nullptr) {
      Json::Value json_resp;
      json_resp["message"] =
          "No model field found or too many files in request body";
      auto resp = cortex_utils::nitroHttpJsonResponse(json_resp);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }
    en->CreateTranscription(req_body,
                            [&q](Json::Value status, Json::Value res) {
                              q.push(std::make_pair(status, res));
                            });
    LOG_TRACE << "Wait to transcript";
    ProcessNonStreamRes(std::move(callback), q);
  } else {
    Json::Value res;
    res["message"] = "Method is not supported yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
    LOG_WARN << "Method is not supported yet";
  }
  LOG_TRACE << "Done transcript";
}

void server::CreateTranslation(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto engine_type =
      (*(req->getJsonObject())).get("engine", kAudioEngine).asString();
  if (!IsEngineLoaded(engine_type)) {
    Json::Value res;
    res["message"] = "Engine is not loaded yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k409Conflict);
    callback(resp);
    LOG_WARN << "Engine is not loaded yet";
    return;
  }

  LOG_TRACE << "Start translate";
  SyncQueue q;
  auto& en = std::get<EngineI*>(engines_[engine_type].engine);
  if (en->IsSupported("CreateTranscription")) {
    auto req_body = ToAudioReqBody(req);
    if (req_body == nullptr) {
      Json::Value json_resp;
      json_resp["message"] =
          "No model field found or too many files in request body";
      auto resp = cortex_utils::nitroHttpJsonResponse(json_resp);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
      return;
    }
    en->CreateTranscription(req_body,
                            [&q](Json::Value status, Json::Value res) {
                              q.push(std::make_pair(status, res));
                            });
    LOG_TRACE << "Wait to translate";
    ProcessNonStreamRes(std::move(callback), q);
  } else {
    Json::Value res;
    res["message"] = "Method is not supported yet";
    auto resp = cortex_utils::nitroHttpJsonResponse(res);
    resp->setStatusCode(k500InternalServerError);
    callback(resp);
    LOG_WARN << "Method is not supported yet";
  }
  LOG_TRACE << "Done translate";
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

bool server::IsEngineLoaded(const std::string& e) {
  return engines_.find(e) != engines_.end();
}

void server::UnloadEngines(const std::string& e) {
  // We unload all engines except:
  // - python engine
  // - llama engine in case we'd like to load audio engine
  // - audio engine in case we'd like to load llama engine
  for (auto it = engines_.begin(); it != engines_.end();) {
    if ((it->first == kPythonRuntimeEngine) ||
        (e == kLlamaEngine && it->first == kAudioEngine) ||
        (e == kAudioEngine && it->first == kLlamaEngine)) {
      it++;
    } else {
      it = engines_.erase(it);
    }
  }
}

std::shared_ptr<Json::Value> server::ToAudioReqBody(const HttpRequestPtr& req) {
  auto req_body = std::make_shared<Json::Value>();
  MultiPartParser part_parser;
  if (part_parser.parse(req) != 0 || part_parser.getFiles().size() != 1) {
    LOG_ERROR << "Must have exactly one file";
    return nullptr;
  }

  auto& file = part_parser.getFiles()[0];
  const auto& form_fields = part_parser.getParameters();

  if (form_fields.find("model") == form_fields.end()) {
    LOG_ERROR << "No model field found in request body";
    return nullptr;
  }

  (*req_body)["model"] = form_fields.at("model");
  // Parse all other optional parameters from the request
  (*req_body)["language"] = form_fields.find("language") != form_fields.end()
                                ? form_fields.at("language")
                                : "en";
  (*req_body)["prompt"] = form_fields.find("prompt") != form_fields.end()
                              ? form_fields.at("prompt")
                              : "";
  (*req_body)["response_format"] =
      form_fields.find("response_format") != form_fields.end()
          ? form_fields.at("response_format")
          : "json";
  (*req_body)["temperature"] =
      form_fields.find("temperature") != form_fields.end()
          ? std::stof(form_fields.at("temperature"))
          : 0;

  // Save input file to temp location
  std::string temp_dir =
      std::filesystem::temp_directory_path().string() + "/" +
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  // Create the directory
  std::filesystem::create_directory(temp_dir);
  // Save the file to the directory, with its original name
  std::string temp_file_path = temp_dir + "/" + file.getFileName();
  file.saveAs(temp_file_path);

  (*req_body)["file"] = temp_file_path;
  return req_body;
}

}  // namespace inferences
