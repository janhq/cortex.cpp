#include "chat_completion_cmd.h"
#include "config/yaml_config.h"
#include "cortex_upd_cmd.h"
#include "database/models.h"
#include "httplib.h"
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

}  // namespace

struct ChunkParser {
  std::string content;
  bool is_done = false;

  ChunkParser(const char* data, size_t data_length) {
    if (data && data_length > kMinDataChunkSize) {
      std::string s(data + kMinDataChunkSize, data_length - kMinDataChunkSize);
      if (s.find("[DONE]") != std::string::npos) {
        is_done = true;
      } else {
        try {
          content =
              json_helper::ParseJsonString(s)["choices"][0]["delta"]["content"]
                  .asString();
        } catch (const std::exception& e) {
          CTL_WRN("JSON parse error: " << e.what());
        }
      }
    }
  }
};

void ChatCompletionCmd::Exec(const std::string& host, int port,
                             const std::string& model_handle, std::string msg) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;
  cortex::db::Models modellist_handler;
  config::YamlHandler yaml_handler;
  try {
    auto model_entry = modellist_handler.GetModelInfo(model_handle);
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
  {
    if (!commands::IsServerAlive(host, port)) {
      CLI_LOG("Server is not started yet, please run `"
              << commands::GetCortexBinary() << " start` to start server!");
      return;
    }
  }

  // Only check if llamacpp engine
  if ((mc.engine.find(kLlamaEngine) != std::string::npos ||
       mc.engine.find(kLlamaRepo) != std::string::npos) &&
      !commands::ModelStatusCmd(model_service_)
           .IsLoaded(host, port, model_handle)) {
    CLI_LOG("Model is not loaded yet!");
    return;
  }

  // Interactive mode or not
  bool interactive = msg.empty();

  // Some instruction for user here
  if (interactive) {
    std::cout << "Inorder to exit, type `exit()`" << std::endl;
  }
  // Model is loaded, start to chat
  {
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
        httplib::Client cli(address);
        Json::Value json_data = mc.ToJson();
        Json::Value new_data;
        new_data["role"] = kUser;
        new_data["content"] = user_input;
        histories_.push_back(std::move(new_data));
        json_data["engine"] = mc.engine;
        Json::Value msgs_array(Json::arrayValue);
        for (const auto& m : histories_) {
          msgs_array.append(m);
        }
        json_data["messages"] = msgs_array;
        json_data["model"] = model_handle;
        //TODO: support non-stream
        json_data["stream"] = true;
        auto data_str = json_data.toStyledString();
        // std::cout << data_str << std::endl;
        cli.set_read_timeout(std::chrono::seconds(60));
        // std::cout << "> ";
        httplib::Request req;
        req.headers = httplib::Headers();
        req.set_header("Content-Type", "application/json");
        req.method = "POST";
        req.path = "/v1/chat/completions";
        req.body = data_str;
        std::string ai_chat;
        req.content_receiver = [&](const char* data, size_t data_length,
                                   uint64_t offset, uint64_t total_length) {
          ChunkParser cp(data, data_length);
          if (cp.is_done) {
            std::cout << std::endl;
            return false;
          }
          std::cout << cp.content << std::flush;
          ai_chat += cp.content;
          return true;
        };
        cli.send(req);

        Json::Value ai_res;
        ai_res["role"] = kAssistant;
        ai_res["content"] = ai_chat;
        histories_.push_back(std::move(ai_res));
      }
      // std::cout << "ok Done" << std::endl;
    } while (interactive);
  }
}

};  // namespace commands
