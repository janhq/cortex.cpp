#include "local_engine.h"
#include <random>
#include <thread>
#include <unordered_set>
#include "utils/curl_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/process/utils.h"
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

struct OaiInfo {
  std::string model;
  bool include_usage = false;
  bool oai_endpoint = false;
  int n_probs = 0;
};

struct StreamingCallback {
  std::shared_ptr<http_callback> callback;
  bool need_stop = true;
  OaiInfo oi;
};

struct Usage {
  int prompt_tokens = 0;
  int completion_tokens = 0;
};

std::string GenerateRandomString(std::size_t length) {
  const std::string characters =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::random_device rd;
  std::mt19937 generator(rd());

  std::uniform_int_distribution<> distribution(
      0, static_cast<int>(characters.size()) - 1);

  std::string random_string(length, '\0');
  std::generate_n(random_string.begin(), length,
                  [&]() { return characters[distribution(generator)]; });

  return random_string;
}

std::vector<int> GetUTF8Bytes(const std::string& str) {
  std::vector<int> bytes;
  for (unsigned char c : str) {
    bytes.push_back(static_cast<int>(c));
  }
  return bytes;
}

Json::Value TransformLogProbs(const Json::Value& logprobs) {
  Json::Value root;
  Json::Value logprobs_json(Json::arrayValue);

  // Iterate through each token group in the input
  for (const auto& token_group : logprobs) {
    Json::Value content_item;

    // Set the token (content)
    content_item["token"] = token_group["content"].asString();

    // Get the probabilities array
    const auto& probs = token_group["probs"];

    // Set the main token's logprob (first probability)
    if (!probs.empty()) {
      content_item["logprob"] = std::log(
          probs[0]["prob"].asDouble() + std::numeric_limits<double>::epsilon());
    }

    // Get UTF-8 bytes for the token
    auto bytes = GetUTF8Bytes(token_group["content"].asString());
    Json::Value bytes_array(Json::arrayValue);
    for (int byte : bytes) {
      bytes_array.append(byte);
    }
    content_item["bytes"] = bytes_array;

    // Create top_logprobs array
    Json::Value top_logprobs(Json::arrayValue);
    for (const auto& prob_item : probs) {
      Json::Value logprob_item;
      logprob_item["token"] = prob_item["tok_str"].asString();
      logprob_item["logprob"] =
          std::log(prob_item["prob"].asDouble() +
                   std::numeric_limits<double>::epsilon());

      // Get UTF-8 bytes for this alternative token
      auto alt_bytes = GetUTF8Bytes(prob_item["tok_str"].asString());
      Json::Value alt_bytes_array(Json::arrayValue);
      for (int byte : alt_bytes) {
        alt_bytes_array.append(byte);
      }
      logprob_item["bytes"] = alt_bytes_array;

      top_logprobs.append(logprob_item);
    }
    content_item["top_logprobs"] = top_logprobs;

    logprobs_json.append(content_item);
  }
  root["content"] = logprobs_json;
  return root;
}

std::string CreateReturnJson(
    const std::string& id, const std::string& model, const std::string& content,
    Json::Value finish_reason, bool include_usage,
    std::optional<Usage> usage = std::nullopt,
    std::optional<Json::Value> logprobs = std::nullopt) {
  Json::Value root;

  root["id"] = id;
  root["model"] = model;
  root["created"] = static_cast<int>(std::time(nullptr));
  root["object"] = "chat.completion.chunk";

  Json::Value choicesArray(Json::arrayValue);
  // If usage, the choices field will always be an empty array
  if (!usage) {
    Json::Value choice;

    choice["index"] = 0;
    Json::Value delta;
    delta["content"] = content;
    delta["role"] = "assistant";
    choice["delta"] = delta;
    choice["finish_reason"] = finish_reason;
    if (logprobs.has_value() && !logprobs.value().empty()) {
      choice["logprobs"] = TransformLogProbs(logprobs.value());
    }

    choicesArray.append(choice);
  }
  root["choices"] = choicesArray;
  if (include_usage) {
    if (usage) {
      Json::Value usage_json;
      Json::Value details;
      details["reasoning_tokens"] = 0;
      usage_json["prompt_tokens"] = (*usage).prompt_tokens;
      usage_json["completion_tokens"] = (*usage).completion_tokens;
      usage_json["total_tokens"] =
          (*usage).prompt_tokens + (*usage).completion_tokens;
      usage_json["completion_tokens_details"] = details;
      root["usage"] = usage_json;
    } else {
      root["usage"] = Json::Value();
    }
  }

  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";  // This sets the indentation to an empty string,
  // producing compact output.
  return Json::writeString(writer, root);
}

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* sc = static_cast<StreamingCallback*>(userdata);
  size_t data_length = size * nmemb;

  if (ptr && data_length > kMinDataChunkSize) {
    std::string chunk(ptr + kMinDataChunkSize, data_length - kMinDataChunkSize);
    CTL_DBG(chunk);
    if (sc->oi.oai_endpoint) {
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
      if (!sc->oi.include_usage &&
          chunk.find("completion_tokens") != std::string::npos) {
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
    } else {
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
      auto json_data = json_helper::ParseJsonString(chunk);
      // DONE
      if (!json_data.isNull() && json_data.isMember("timings")) {
        std::optional<Usage> u;
        if (sc->oi.include_usage) {
          u = Usage{json_data["tokens_evaluated"].asInt(),
                    json_data["tokens_predicted"].asInt()};
        }

        Json::Value chunk_json;
        chunk_json["data"] =
            "data: " + CreateReturnJson(GenerateRandomString(20), sc->oi.model,
                                        "", "stop", sc->oi.include_usage, u);
        Json::Value status;
        status["is_done"] = false;
        status["has_error"] = false;
        status["is_stream"] = true;
        status["status_code"] = 200;
        (*sc->callback)(std::move(status), std::move(chunk_json));

        sc->need_stop = false;
        return data_length;
      }

      Json::Value logprobs;
      if (sc->oi.n_probs > 0) {
        logprobs = json_data["completion_probabilities"];
      }
      std::string to_send;
      if (json_data.isMember("choices") && json_data["choices"].isArray() &&
          json_data["choices"].size() > 0) {
        to_send = json_data["choices"][0].get("text", "").asString();
      }
      CTL_DBG(to_send);
      const std::string str =
          CreateReturnJson(GenerateRandomString(20), sc->oi.model, to_send, "",
                           sc->oi.include_usage, std::nullopt, logprobs);
      Json::Value chunk_json;
      chunk_json["data"] = "data: " + str;
      Json::Value status;
      status["is_done"] = false;
      status["has_error"] = false;
      status["is_stream"] = true;
      status["status_code"] = 200;
      (*sc->callback)(std::move(status), std::move(chunk_json));
      return data_length;
    }
  }

  return data_length;
}

Json::Value ConvertLogitBiasToArray(const Json::Value& input) {
  Json::Value result(Json::arrayValue);
  if (input.isObject()) {
    const auto& member_names = input.getMemberNames();
    for (const auto& tokenStr : member_names) {
      Json::Value pair(Json::arrayValue);
      pair.append(std::stoi(tokenStr));
      pair.append(input[tokenStr].asFloat());
      result.append(pair);
    }
  }
  return result;
}

Json::Value CreateFullReturnJson(
    const std::string& id, const std::string& model, const std::string& content,
    const std::string& system_fingerprint, int prompt_tokens,
    int completion_tokens, Json::Value finish_reason = Json::Value(),
    std::optional<Json::Value> logprobs = std::nullopt) {
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
  if (logprobs.has_value() && !logprobs.value().empty()) {
    choice["logprobs"] = TransformLogProbs(logprobs.value());
  }

  choicesArray.append(choice);
  root["choices"] = choicesArray;

  Json::Value usage;
  usage["prompt_tokens"] = prompt_tokens;
  usage["completion_tokens"] = completion_tokens;
  usage["total_tokens"] = prompt_tokens + completion_tokens;
  root["usage"] = usage;

  return root;
}

}  // namespace

LocalEngine::~LocalEngine() {
  for (auto& [_, si] : server_map_) {
    (void)cortex::process::KillProcess(si.process_info);
  }
  server_map_.clear();
}
void LocalEngine::HandleChatCompletion(std::shared_ptr<Json::Value> json_body,
                                       http_callback&& callback) {
  auto model_id = json_body->get("model", "").asString();
  if (model_id.empty()) {
    CTL_WRN("Model is empty");
  }
  if (server_map_.find(model_id) != server_map_.end()) {
    auto& s = server_map_[model_id];
    auto oaicompat = [&json_body]() -> bool {
      if (json_body->isMember("logprobs") &&
          (*json_body)["logprobs"].asBool()) {
        return false;
      }
      return true;
    }();
    if (oaicompat) {
      HandleOpenAiChatCompletion(
          json_body, const_cast<http_callback&&>(callback), model_id);
    } else {
      HandleNonOpenAiChatCompletion(
          json_body, const_cast<http_callback&&>(callback), model_id);
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

void LocalEngine::HandleEmbedding(std::shared_ptr<Json::Value> json_body,
                                  http_callback&& callback) {
  auto model_id = json_body->get("model", "").asString();
  if (model_id.empty()) {
    CTL_WRN("Model is empty");
  }
  if (server_map_.find(model_id) != server_map_.end()) {
    auto& s = server_map_[model_id];
    auto url = url_parser::Url{
        /*.protocol*/ "http",
        /*.host*/ s.host + ":" + std::to_string(s.port),
        /*.pathParams*/ {"v1", "embeddings"},
        /* .queries = */ {},
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

void LocalEngine::LoadModel(std::shared_ptr<Json::Value> json_body,
                            http_callback&& callback) {
  CTL_INF("Start loading model");
  auto wait_for_server_up = [this](const std::string& model,
                                   const std::string& host, int port) {
    auto url = url_parser::Url{
        /*.protocol*/ "http",
        /*.host*/ host + ":" + std::to_string(port),
        /*.pathParams*/ {"health"},
        /*.queries*/ {},
    };
    while (server_map_.find(model) != server_map_.end()) {
      auto res = curl_utils::SimpleGet(url.ToFullPath());
      if (res.has_error()) {
        LOG_INFO << "Wait for server up ..";
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
  s.pre_prompt = json_body->get("pre_prompt", "").asString();
  s.user_prompt = json_body->get("user_prompt", "USER: ").asString();
  s.ai_prompt = json_body->get("ai_prompt", "ASSISTANT: ").asString();
  s.system_prompt =
      json_body->get("system_prompt", "ASSISTANT's RULE: ").asString();
  std::vector<std::string> params = ConvertJsonToParamsVector(*json_body);
  params.push_back("--host");
  params.push_back(s.host);
  params.push_back("--port");
  params.push_back(std::to_string(s.port));

  params.push_back("--pooling");
  params.push_back("mean");
  params.push_back("--jinja");

  std::vector<std::string> v;
  v.reserve(params.size() + 1);
  auto engine_dir = engine_service_.GetEngineDirPath(kLlamaRepo);
  if (engine_dir.has_error()) {
    CTL_WRN(engine_dir.error());
    server_map_.erase(model_id);
    return;
  }
  auto exe = (engine_dir.value().first / kLlamaServer).string();

  v.push_back(exe);
  v.insert(v.end(), params.begin(), params.end());
  engine_service_.RegisterEngineLibPath();

  auto log_path =
      (file_manager_utils::GetCortexLogPath() / "logs" / "cortex.log").string();
  CTL_DBG("log: " << log_path);
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
  if (wait_for_server_up(model_id, s.host, s.port)) {
    s.start_time = std::chrono::system_clock::now().time_since_epoch() /
                   std::chrono::milliseconds(1);
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

void LocalEngine::UnloadModel(std::shared_ptr<Json::Value> json_body,
                              http_callback&& callback) {
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

void LocalEngine::GetModelStatus(std::shared_ptr<Json::Value> json_body,
                                 http_callback&& callback) {
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

void LocalEngine::GetModels(std::shared_ptr<Json::Value> json_body,
                            http_callback&& callback) {
  Json::Value json_resp;
  Json::Value model_array(Json::arrayValue);
  {
    for (const auto& [m, s] : server_map_) {
      Json::Value val;
      val["id"] = m;
      val["engine"] = kLlamaEngine;
      val["start_time"] = s.start_time;
      val["model_size"] = 0u;
      val["vram"] = 0u;
      val["ram"] = 0u;
      val["object"] = "model";
      model_array.append(val);
    }
  }

  json_resp["object"] = "list";
  json_resp["data"] = model_array;

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = 200;
  callback(std::move(status), std::move(json_resp));
  CTL_INF("Running models responded");
  (void)json_body;
}

void LocalEngine::HandleOpenAiChatCompletion(
    std::shared_ptr<Json::Value> json_body, http_callback&& callback,
    const std::string& model) {
  CTL_DBG("Hanle OpenAI chat completion");
  auto is_stream = (*json_body).get("stream", false).asBool();
  auto include_usage = [&json_body, is_stream]() -> bool {
    if (is_stream) {
      if (json_body->isMember("stream_options") &&
          !(*json_body)["stream_options"].isNull()) {
        return (*json_body)["stream_options"]
            .get("include_usage", false)
            .asBool();
      }
      return false;
    }
    return false;
  }();

  auto n = [&json_body, is_stream]() -> int {
    if (is_stream)
      return 1;
    return (*json_body).get("n", 1).asInt();
  }();

  auto& s = server_map_.at(model);
  // Format logit_bias
  if (json_body->isMember("logit_bias")) {
    auto logit_bias = ConvertLogitBiasToArray((*json_body)["logit_bias"]);
    (*json_body)["logit_bias"] = logit_bias;
  }
  // llama.cpp server only supports n = 1
  (*json_body)["n"] = 1;

  auto url = url_parser::Url{
      /*.protocol*/ "http",
      /*.host*/ s.host + ":" + std::to_string(s.port),
      /*.pathParams*/ {"v1", "chat", "completions"},
      /*.queries*/ {},
  };

  if (is_stream) {
    q_.RunInQueue([s, json_body, callback, model, url = std::move(url)] {
      auto curl = curl_easy_init();
      if (!curl) {
        CTL_WRN("Failed to initialize CURL");
        return;
      }

      curl_easy_setopt(curl, CURLOPT_URL, url.ToFullPath().c_str());
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      CTL_INF(url.ToFullPath());

      struct curl_slist* headers = nullptr;
      headers = curl_slist_append(headers, "Content-Type: application/json");
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      auto json_str = json_body->toStyledString();
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_str.length());
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

      StreamingCallback sc;
      OaiInfo oi{model, false /*include_usage*/, true /*oai_endpoint*/,
                 0 /*n_probs*/};
      sc.callback = std::make_shared<http_callback>(callback);
      sc.need_stop = true;
      sc.oi = oi;

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
    Json::Value result;
    // multiple choices
    for (int i = 0; i < n; i++) {
      auto response = curl_utils::SimplePostJson(url.ToFullPath(),
                                                 json_body->toStyledString());

      if (response.has_value()) {
        auto r = response.value();
        if (i == 0) {
          result = r;
        } else {
          r["choices"][0]["index"] = i;
          result["choices"].append(r["choices"][0]);
          result["usage"]["completion_tokens"] =
              result["usage"]["completion_tokens"].asInt() +
              r["usage"]["completion_tokens"].asInt();
          result["usage"]["prompt_tokens"] =
              result["usage"]["prompt_tokens"].asInt() +
              r["usage"]["prompt_tokens"].asInt();
          result["usage"]["total_tokens"] =
              result["usage"]["total_tokens"].asInt() +
              r["usage"]["total_tokens"].asInt();
        }

        if (i == n - 1) {
          Json::Value status;
          status["is_done"] = true;
          status["has_error"] = false;
          status["is_stream"] = false;
          status["status_code"] = 200;
          callback(std::move(status), std::move(result));
        }
      } else {
        CTL_WRN("Error: " << response.error());
        Json::Value status;
        status["is_done"] = true;
        status["has_error"] = true;
        status["is_stream"] = false;
        status["status_code"] = 500;
        callback(std::move(status), std::move(response.value()));
        break;
      }
    }
  }
}

// (sang) duplicate code but it is easier to clean when
// llama-server upstream is fully OpenAI API Compatible
void LocalEngine::HandleNonOpenAiChatCompletion(
    std::shared_ptr<Json::Value> json_body, http_callback&& callback,
    const std::string& model) {
  CTL_DBG("Hanle NonOpenAI chat completion");
  auto is_stream = (*json_body).get("stream", false).asBool();
  auto include_usage = [&json_body, is_stream]() -> bool {
    if (is_stream) {
      if (json_body->isMember("stream_options") &&
          !(*json_body)["stream_options"].isNull()) {
        return (*json_body)["stream_options"]
            .get("include_usage", false)
            .asBool();
      }
      return false;
    }
    return false;
  }();

  auto n = [&json_body, is_stream]() -> int {
    if (is_stream)
      return 1;
    return (*json_body).get("n", 1).asInt();
  }();

  auto& s = server_map_.at(model);

  // Format logit_bias
  if (json_body->isMember("logit_bias")) {
    auto logit_bias = ConvertLogitBiasToArray((*json_body)["logit_bias"]);
    (*json_body)["logit_bias"] = logit_bias;
  }
  auto get_message = [](const Json::Value& msg_content) -> std::string {
    if (msg_content.isArray()) {
      for (const auto& mc : msg_content) {
        if (mc["type"].asString() == "text") {
          return mc["text"].asString();
        }
      }
    } else {
      return msg_content.asString();
    }
    return "";
  };

  if (!json_body->isMember("prompt") ||
      (*json_body)["prompt"].asString().empty()) {
    auto formatted_output = s.pre_prompt;
    for (const auto& message : (*json_body)["messages"]) {
      auto input_role = message["role"].asString();
      std::string role;
      if (input_role == "user") {
        role = s.user_prompt;
      } else if (input_role == "assistant") {
        role = s.ai_prompt;
      } else if (input_role == "system") {
        role = s.system_prompt;
      } else {
        role = input_role;
      }

      if (auto content = get_message(message["content"]); !content.empty()) {
        formatted_output += role + content;
      }
    }
    formatted_output += s.ai_prompt;
    (*json_body)["prompt"] = formatted_output;
  }

  (*json_body)["n"] = 1;
  int n_probs = json_body->get("n_probs", 0).asInt();

  auto url = url_parser::Url{
      /*.protocol*/ "http",
      /*.host*/ s.host + ":" + std::to_string(s.port),
      /*.pathParams*/ {"v1", "completions"},
      /*.queries*/ {},
  };

  if (is_stream) {
    q_.RunInQueue([s, json_body, callback, n_probs, model,
                   url = std::move(url)] {
      auto curl = curl_easy_init();
      if (!curl) {
        CTL_WRN("Failed to initialize CURL");
        return;
      }

      curl_easy_setopt(curl, CURLOPT_URL, url.ToFullPath().c_str());
      curl_easy_setopt(curl, CURLOPT_POST, 1L);

      struct curl_slist* headers = nullptr;
      headers = curl_slist_append(headers, "Content-Type: application/json");
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      auto json_str = json_body->toStyledString();
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_str.length());
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

      StreamingCallback sc;
      OaiInfo oi{model, false /*include_usage*/, false /*oai_endpoint*/,
                 n_probs};
      sc.callback = std::make_shared<http_callback>(callback);
      sc.need_stop = true;
      sc.oi = oi;

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

    Json::Value result;
    int prompt_tokens = 0;
    int predicted_tokens = 0;
    // multiple choices
    for (int i = 0; i < n; i++) {
      auto response = curl_utils::SimplePostJson(url.ToFullPath(),
                                                 json_body->toStyledString());
      if (response.has_value()) {
        auto r = response.value();
        Json::Value logprobs;
        prompt_tokens += r["tokens_evaluated"].asInt();
        predicted_tokens += r["tokens_predicted"].asInt();
        std::string to_send = r["content"].asString();
        string_utils::LTrim(to_send);
        if (n_probs > 0) {
          logprobs = r["completion_probabilities"];
        }
        if (i == 0) {
          result = CreateFullReturnJson(
              GenerateRandomString(20), model, to_send, "_", prompt_tokens,
              predicted_tokens, Json::Value("stop"), logprobs);
        } else {
          auto choice = CreateFullReturnJson(
              GenerateRandomString(20), model, to_send, "_", prompt_tokens,
              predicted_tokens, Json::Value("stop"), logprobs)["choices"][0];
          choice["index"] = i;
          result["choices"].append(choice);
          result["usage"]["completion_tokens"] = predicted_tokens;
          result["usage"]["prompt_tokens"] = prompt_tokens;
          result["usage"]["total_tokens"] = predicted_tokens + prompt_tokens;
        }

        if (i == n - 1) {
          Json::Value status;
          status["is_done"] = true;
          status["has_error"] = false;
          status["is_stream"] = false;
          status["status_code"] = 200;
          callback(std::move(status), std::move(result));
        }
      } else {
        CTL_WRN("Error: " << response.error());
        Json::Value status;
        status["is_done"] = true;
        status["has_error"] = true;
        status["is_stream"] = false;
        status["status_code"] = 500;
        callback(std::move(status), std::move(response.value()));
        break;
      }
    }
  }
}

}  // namespace cortex::local
