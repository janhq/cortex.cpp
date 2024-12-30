#include "chat_completion_cmd.h"
#include <curl/curl.h>
#include "config/yaml_config.h"
#include "cortex_upd_cmd.h"
#include "database/models.h"
#include "model_status_cmd.h"
#include "server_start_cmd.h"
#include "utils/engine_constants.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

namespace commands {
namespace {
constexpr const char* kExitChat = "exit()";
constexpr const auto kMinDataChunkSize = 6u;
constexpr const char* kUser = "user";
constexpr const char* kAssistant = "assistant";

struct StreamingCallback {
  std::string* ai_chat;
  bool is_done;

  StreamingCallback() : ai_chat(nullptr), is_done(false) {}
};

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* callback = static_cast<StreamingCallback*>(userdata);
  size_t data_length = size * nmemb;

  if (ptr && data_length > kMinDataChunkSize) {
    std::string chunk(ptr + kMinDataChunkSize, data_length - kMinDataChunkSize);
    if (chunk.find("[DONE]") != std::string::npos) {
      callback->is_done = true;
      std::cout << std::endl;
      return data_length;
    }

    try {
      std::string content =
          json_helper::ParseJsonString(chunk)["choices"][0]["delta"]["content"]
              .asString();
      std::cout << content << std::flush;
      if (callback->ai_chat) {
        *callback->ai_chat += content;
      }
    } catch (const std::exception& e) {
      CTL_WRN("JSON parse error: " << e.what());
    }
  }

  return data_length;
}
}  // namespace

void ChatCompletionCmd::Exec(const std::string& host, int port,
                             const std::string& model_handle, std::string msg) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = db_service_->GetModelInfo(model_handle);
    if (model_entry.has_error()) {
      CLI_LOG("Error: " + model_entry.error());
      return;
    }
    yaml_handler.ModelConfigFromFile(
        fmu::ToAbsoluteCortexDataPath(
            fs::path(model_entry.value().path_to_model_yaml))
            .string());
    auto mc = yaml_handler.GetModelConfig();
    Exec(host, port, model_handle, mc, std::move(msg));
  } catch (const std::exception& e) {
    CLI_LOG("Fail to start model information with ID '" + model_handle +
            "': " + e.what());
  }
}

void ChatCompletionCmd::Exec(const std::string& host, int port,
                             const std::string& model_handle,
                             const config::ModelConfig& mc, std::string msg) {
  auto address = host + ":" + std::to_string(port);

  // Check if server is started
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Server is not started yet, please run `"
            << commands::GetCortexBinary() << " start` to start server!");
    return;
  }

  // Only check if llamacpp engine
  if ((mc.engine.find(kLlamaEngine) != std::string::npos ||
       mc.engine.find(kLlamaRepo) != std::string::npos) &&
      !commands::ModelStatusCmd().IsLoaded(host, port, model_handle)) {
    CLI_LOG("Model is not loaded yet!");
    return;
  }

  auto curl = curl_easy_init();
  if (!curl) {
    CLI_LOG("Failed to initialize CURL");
    return;
  }

  auto url = "http://" + address + "/v1/chat/completions";
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);

  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  // Interactive mode or not
  bool interactive = msg.empty();

  if (interactive) {
    std::cout << "In order to exit, type `exit()`" << std::endl;
  }

  do {
    std::string user_input = std::move(msg);
    if (user_input.empty()) {
      std::cout << "> ";
      if (!std::getline(std::cin, user_input)) {
        break;
      }
    }

    string_utils::Trim(user_input);
    if (user_input == kExitChat) {
      break;
    }

    if (!user_input.empty()) {
      // Prepare JSON payload
      Json::Value new_data;
      new_data["role"] = kUser;
      new_data["content"] = user_input;
      histories_.push_back(std::move(new_data));

      Json::Value json_data = mc.ToJson();
      json_data["engine"] = mc.engine;

      Json::Value msgs_array(Json::arrayValue);
      for (const auto& m : histories_) {
        msgs_array.append(m);
      }

      json_data["messages"] = msgs_array;
      json_data["model"] = model_handle;
      json_data["stream"] = true;

      auto json_str = json_data.toStyledString();
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_str.length());
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

      std::string ai_chat;
      StreamingCallback callback;
      callback.ai_chat = &ai_chat;

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &callback);
      auto res = curl_easy_perform(curl);

      if (res != CURLE_OK) {
        CLI_LOG("CURL request failed: " << curl_easy_strerror(res));
      } else {
        Json::Value ai_res;
        ai_res["role"] = kAssistant;
        ai_res["content"] = ai_chat;
        histories_.push_back(std::move(ai_res));
      }
    }
  } while (interactive);

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
}
}  // namespace commands
