#include "local_engine.h"
#include <random>
#include <thread>
#include <unordered_set>
#include "utils/curl_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace cortex::local {

namespace {
const std::unordered_set<std::string> kIgnoredParams = {
    "model",        "model_alias",     "embedding",  "ai_prompt",
    "ai_template",  "prompt_template", "mmproj",     "system_prompt",
    "created",      "stream",          "name",       "os",
    "owned_by",     "files",           "gpu_arch",   "quantization_method",
    "engine",       "system_template", "max_tokens", "user_template",
    "user_prompt",  "min_keep",        "mirostat",   "mirostat_eta",
    "mirostat_tau", "text_model",      "version",    "n_probs",
    "object",       "penalize_nl",     "precision",  "size",
    "stop",         "tfs_z",           "typ_p"};

const std::unordered_map<std::string, std::string> kParamsMap = {
    {"cpu_threads", "--threads"},
    {"n_ubatch", "--ubatch-size"},
    {"n_batch", "--batch-size"},
    {"n_parallel", "--parallel"},
    {"temperature", "--temp"},
    {"top_k", "--top-k"},
    {"top_p", "--top-p"},
    {"min_p", "--min-p"},
    {"dynatemp_exponent", "--dynatemp-exp"},
    {"ctx_len", "--ctx-size"},
    {"ngl", "-ngl"},
};

int GenerateRandomInteger(int min, int max) {
  static std::random_device rd;   // Seed for the random number engine
  static std::mt19937 gen(rd());  // Mersenne Twister random number engine
  std::uniform_int_distribution<> dis(
      min, max);  // Distribution for the desired range

  return dis(gen);  // Generate and return a random integer within the range
}

std::vector<std::string> ConvertJsonToParamsVector(const Json::Value& root) {
  std::vector<std::string> res;
  std::string errors;

  for (const auto& member : root.getMemberNames()) {
    if (member == "model_path" || member == "llama_model_path") {
      if (!root[member].isNull()) {
        res.push_back("--model");
        res.push_back(root[member].asString());
      }
      continue;
    } else if (kIgnoredParams.find(member) != kIgnoredParams.end()) {
      continue;
    } else if (kParamsMap.find(member) != kParamsMap.end()) {
      res.push_back(kParamsMap.at(member));
      res.push_back(root[member].asString());
      continue;
    } else if (member == "model_type") {
      if (root[member].asString() == "embedding") {
        res.push_back("--embedding");
      }
      continue;
    }

    res.push_back("--" + member);
    if (root[member].isString()) {
      res.push_back(root[member].asString());
    } else if (root[member].isInt()) {
      res.push_back(std::to_string(root[member].asInt()));
    } else if (root[member].isDouble()) {
      res.push_back(std::to_string(root[member].asDouble()));
    } else if (root[member].isArray()) {
      std::stringstream ss;
      ss << "[";
      bool first = true;
      for (const auto& value : root[member]) {
        if (!first) {
          ss << ", ";
        }
        ss << "\"" << value.asString() << "\"";
        first = false;
      }
      ss << "] ";
      res.push_back(ss.str());
    }
  }

  return res;
}

constexpr const auto kMinDataChunkSize = 6u;

struct StreamingCallback {
  std::shared_ptr<std::function<void(Json::Value&&, Json::Value&&)>> callback;
  bool need_stop = true;
};

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* sc = static_cast<StreamingCallback*>(userdata);
  size_t data_length = size * nmemb;

  if (ptr && data_length > kMinDataChunkSize) {
    std::string chunk(ptr + kMinDataChunkSize, data_length - kMinDataChunkSize);
    CTL_DBG(chunk);
    if (chunk.find("[DONE]") != std::string::npos) {
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = false;
      status["is_stream"] = true;
      status["status_code"] = 200;
      Json::Value chunk_json;
      chunk_json["data"] = "data: [DONE]";
      sc->need_stop = false;
      (*sc->callback)(std::move(status), std::move(chunk_json));
      return data_length;
    }
    Json::Value chunk_json;
    chunk_json["data"] = "data: " + chunk;
    Json::Value status;
    status["is_done"] = false;
    status["has_error"] = false;
    status["is_stream"] = true;
    status["status_code"] = 200;
    (*sc->callback)(std::move(status), std::move(chunk_json));
  }

  return data_length;
}
}  // namespace

LocalEngine::~LocalEngine() {
  for (auto& [_, si] : server_map_) {
    (void) cortex::process::KillProcess(si.process_info);
  }
  server_map_.clear();
}
void LocalEngine::HandleChatCompletion(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  auto model_id = json_body->get("model", "").asString();
  if (model_id.empty()) {
    CTL_WRN("Model is empty");
  }
  if (server_map_.find(model_id) != server_map_.end()) {
    auto& s = server_map_[model_id];
    bool is_stream = json_body->get("stream", false).asBool();
    if (is_stream) {
      q_.RunInQueue([s, json_body, callback] {
        auto curl = curl_easy_init();
        if (!curl) {
          CTL_WRN("Failed to initialize CURL");
          return;
        }

        auto url = "http://" + s.host + ":" + std::to_string(s.port) +
                   "/v1/chat/completions";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        auto json_str = json_body->toStyledString();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_str.length());
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        StreamingCallback sc;
        sc.callback =
            std::make_shared<std::function<void(Json::Value&&, Json::Value&&)>>(
                callback);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sc);
        auto res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
          CTL_WRN("CURL request failed: " << curl_easy_strerror(res));

          Json::Value status;
          status["is_done"] = true;
          status["has_error"] = true;
          status["is_stream"] = true;
          status["status_code"] = 500;

          Json::Value error;
          error["error"] = curl_easy_strerror(res);
          callback(std::move(status), std::move(error));
        }
        curl_easy_cleanup(curl);
        if (sc.need_stop) {
          CTL_DBG("No stop message received, need to stop");
          Json::Value status;
          status["is_done"] = true;
          status["has_error"] = false;
          status["is_stream"] = true;
          status["status_code"] = 200;
          (*sc.callback)(std::move(status), Json::Value());
        }
      });

    } else {
      auto url = url_parser::Url{
          .protocol = "http",
          .host = s.host + ":" + std::to_string(s.port),
          .pathParams =
              {
                  "v1",
                  "chat",
                  "completions",
              },
      };

      auto response = curl_utils::SimplePostJson(url.ToFullPath(),
                                                 json_body->toStyledString());

      if (response.has_error()) {
        CTL_WRN("Error: " << response.error());
      } else {
        Json::Value status;
        status["is_done"] = true;
        status["has_error"] = false;
        status["is_stream"] = false;
        status["status_code"] = 200;
        callback(std::move(status), std::move(response.value()));
      }
    }
  } else {
    Json::Value error;
    error["error"] = "Model is not loaded yet: " + model_id;
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = 400;
    callback(std::move(status), std::move(error));
  }
}

void LocalEngine::HandleEmbedding(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  auto model_id = json_body->get("model", "").asString();
  if (model_id.empty()) {
    CTL_WRN("Model is empty");
  }
  if (server_map_.find(model_id) != server_map_.end()) {
    auto& s = server_map_[model_id];
    auto url = url_parser::Url{
        .protocol = "http",
        .host = s.host + ":" + std::to_string(s.port),
        .pathParams =
            {
                "v1",
                "embeddings",
            },
    };

    auto response = curl_utils::SimplePostJson(url.ToFullPath(),
                                               json_body->toStyledString());

    if (response.has_error()) {
      CTL_WRN("Error: " << response.error());
      Json::Value error;
      error["error"] = response.error();
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = true;
      status["is_stream"] = false;
      status["status_code"] = 400;
      callback(std::move(status), std::move(error));
    } else {
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = false;
      status["is_stream"] = false;
      status["status_code"] = 200;
      callback(std::move(status), std::move(response.value()));
    }
  } else {
    Json::Value error;
    error["error"] = "Model is not loaded yet: " + model_id;
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = 400;
    callback(std::move(status), std::move(error));
  }
}

void LocalEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  CTL_INF("Start loading model");
  auto wait_for_server_up = [](const std::string& host, int port) {
    auto url = url_parser::Url{
        .protocol = "http",
        .host = host + ":" + std::to_string(port),
        .pathParams = {"health"},
    };
    for (size_t i = 0; i < 10; i++) {
      auto res = curl_utils::SimpleGet(url.ToFullPath());
      if (res.has_error()) {
        LOG_INFO << "Wait for server up: " << i;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      } else {
        return true;
      }
    }
    return false;
  };

  LOG_DEBUG << "Start to spawn llama-server";
  auto model_id = json_body->get("model", "").asString();
  if (model_id.empty()) {
    CTL_WRN("Model is empty");
  }
  server_map_[model_id].host = "127.0.0.1";
  server_map_[model_id].port = GenerateRandomInteger(39400, 39999);
  auto& s = server_map_[model_id];
  std::vector<std::string> params = ConvertJsonToParamsVector(*json_body);
  params.push_back("--host");
  params.push_back(s.host);
  params.push_back("--port");
  params.push_back(std::to_string(s.port));

  params.push_back("--pooling");
  params.push_back("mean");

  std::vector<std::string> v;
  v.reserve(params.size() + 1);
  auto engine_dir = engine_service_.GetEngineDirPath("llama.cpp");
  if (engine_dir.has_error()) {
    CTL_WRN(engine_dir.error());
    server_map_.erase(model_id);
    return;
  }
  auto exe = (engine_dir.value().first / "llama-server").string();

  v.push_back(exe);
  v.insert(v.end(), params.begin(), params.end());
  engine_service_.RegisterEngineLibPath();

  auto log_path =
      (file_manager_utils::GetCortexLogPath() / "logs" / "cortex.log").string();
  CTL_INF("log: " << log_path);
  auto result = cortex::process::SpawnProcess(v, log_path, log_path);
  if (result.has_error()) {
    CTL_ERR("Fail to spawn process. " << result.error());
    Json::Value error;
    error["error"] = "Fail to spawn process";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = 500;
    callback(std::move(status), std::move(error));
    server_map_.erase(model_id);
    return;
  }

  s.process_info = result.value();
  if (wait_for_server_up(s.host, s.port)) {
    Json::Value response;
    response["status"] = "Model loaded successfully with pid: " +
                         std::to_string(s.process_info.pid);
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = 200;
    callback(std::move(status), std::move(response));
  } else {
    server_map_.erase(model_id);
    Json::Value error;
    error["error"] = "Wait for server up timeout";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = 500;
    callback(std::move(status), std::move(error));
  }
}

void LocalEngine::UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  auto model_id = json_body->get("model", "").asString();
  if (model_id.empty()) {
    CTL_WRN("Model is empty");
  }

  if (server_map_.find(model_id) != server_map_.end()) {
    auto& s = server_map_[model_id];
#if defined(_WIN32) || defined(_WIN64)
    auto sent = cortex::process::KillProcess(s.process_info);
#else
    auto sent = (kill(s.process_info.pid, SIGTERM) != -1);
#endif
    if (sent) {
      LOG_INFO << "SIGINT signal sent to child process";
      Json::Value response;
      response["status"] = "Model unloaded successfully with pid: " +
                           std::to_string(s.process_info.pid);
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = false;
      status["is_stream"] = false;
      status["status_code"] = 200;
      callback(std::move(status), std::move(response));
      server_map_.erase(model_id);
    } else {
      LOG_ERROR << "Failed to send SIGINT signal to child process";
      Json::Value error;
      error["error"] = "Failed to unload model: " + model_id;
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = true;
      status["is_stream"] = false;
      status["status_code"] = 500;
      callback(std::move(status), std::move(error));
    }
  } else {
    Json::Value error;
    error["error"] = "Model is not loaded yet: " + model_id;
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = 400;
    callback(std::move(status), std::move(error));
  }
}

void LocalEngine::GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  auto model_id = json_body->get("model", "").asString();
  if (model_id.empty()) {
    CTL_WRN("Model is empty");
  }
  if (server_map_.find(model_id) != server_map_.end()) {
    Json::Value response;
    response["status"] = "Model is loaded";
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = 200;
    callback(std::move(status), std::move(response));
  } else {
    Json::Value error;
    error["error"] = "Model is not loaded yet: " + model_id;
    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = true;
    status["is_stream"] = false;
    status["status_code"] = 400;
    callback(std::move(status), std::move(error));
  }
}

void LocalEngine::GetModels(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {}
}  // namespace cortex::local