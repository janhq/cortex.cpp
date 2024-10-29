#include "model_pull_cmd.h"
#include <memory>
#include "cli/utils/easywsclient.hpp"
#include "cli/utils/indicators.hpp"
#include "common/event.h"
#include "database/models.h"
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/download_progress.h"
#include "utils/format_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/scope_exit.h"
#include "utils/string_utils.h"
#if defined(_WIN32)
#include <signal.h>
#endif

namespace commands {
std::function<void(int)> shutdown_handler;
inline void signal_handler(int signal) {
  if (shutdown_handler) {
    shutdown_handler(signal);
  }
}
std::optional<std::string> ModelPullCmd::Exec(const std::string& host, int port,
                                              const std::string& input) {

  // model_id: use to check the download progress
  // model: use as a parameter for pull API
  auto model_id = input;
  auto model = input;

  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return std::nullopt;
    }
  }

  // Get model info from Server
  httplib::Client cli(host + ":" + std::to_string(port));
  cli.set_read_timeout(std::chrono::seconds(60));
  Json::Value j_data;
  j_data["model"] = input;
  auto d_str = j_data.toStyledString();
  auto res = cli.Post("/models/pull/info", httplib::Headers(), d_str.data(),
                      d_str.size(), "application/json");

  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      // CLI_LOG(res->body);
      auto root = json_helper::ParseJsonString(res->body);
      auto id = root["id"].asString();
      bool is_cortexso = root["modelSource"].asString() == "cortexso";
      auto default_branch = root["defaultBranch"].asString();
      std::vector<std::string> downloaded;
      for (auto const& v : root["downloadedModels"]) {
        downloaded.push_back(v.asString());
      }
      std::vector<std::string> avails;
      for (auto const& v : root["availableModels"]) {
        avails.push_back(v.asString());
      }
      auto download_url = root["downloadUrl"].asString();

      if (downloaded.empty() && avails.empty()) {
        model_id = id;
        model = download_url;
      } else {
        if (is_cortexso) {
          auto selection = cli_selection_utils::PrintModelSelection(
              downloaded, avails,
              default_branch.empty()
                  ? std::nullopt
                  : std::optional<std::string>(default_branch));

          if (!selection.has_value()) {
            CLI_LOG("Invalid selection");
            return std::nullopt;
          }
          model_id = selection.value();
          model = model_id;
        } else {
          auto selection = cli_selection_utils::PrintSelection(avails);
          CLI_LOG("Selected: " << selection.value());
          model_id = id + ":" + selection.value();
          model = download_url + selection.value();
        }
      }
    } else {
      auto root = json_helper::ParseJsonString(res->body);
      CLI_LOG(root["message"].asString());
      return std::nullopt;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return std::nullopt;
  }

  // Send request download model to server
  Json::Value json_data;
  json_data["model"] = model;
  auto data_str = json_data.toStyledString();
  cli.set_read_timeout(std::chrono::seconds(60));
  res = cli.Post("/v1/models/pull", httplib::Headers(), data_str.data(),
                 data_str.size(), "application/json");

  if (res) {
    if (res->status != httplib::StatusCode::OK_200) {
      auto root = json_helper::ParseJsonString(res->body);
      CLI_LOG(root["message"].asString());
      return std::nullopt;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return std::nullopt;
  }

  CLI_LOG("Start downloading ...")
  DownloadProgress dp;
  bool force_stop = false;

  shutdown_handler = [this, &dp, &host, &port, &model_id, &force_stop](int) {
    force_stop = true;
    AbortModelPull(host, port, model_id);
    dp.ForceStop();
  };

  utils::ScopeExit se([]() { shutdown_handler = {}; });
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
  struct sigaction sigint_action;
  sigint_action.sa_handler = signal_handler;
  sigemptyset(&sigint_action.sa_mask);
  sigint_action.sa_flags = 0;
  sigaction(SIGINT, &sigint_action, NULL);
  sigaction(SIGTERM, &sigint_action, NULL);
#elif defined(_WIN32)
  auto console_ctrl_handler = +[](DWORD ctrl_type) -> BOOL {
    return (ctrl_type == CTRL_C_EVENT) ? (signal_handler(SIGINT), true) : false;
  };
  SetConsoleCtrlHandler(
      reinterpret_cast<PHANDLER_ROUTINE>(console_ctrl_handler), true);
#endif
  dp.Connect(host, port);
  if (!dp.Handle(model_id))
    return std::nullopt;
  if (force_stop)
    return std::nullopt;
  CLI_LOG("Model " << model_id << " downloaded successfully!")
  return model_id;
}

bool ModelPullCmd::AbortModelPull(const std::string& host, int port,
                                  const std::string& task_id) {
  Json::Value json_data;
  json_data["taskId"] = task_id;
  auto data_str = json_data.toStyledString();
  httplib::Client cli(host + ":" + std::to_string(port));
  cli.set_read_timeout(std::chrono::seconds(60));
  auto res = cli.Delete("/v1/models/pull", httplib::Headers(), data_str.data(),
                        data_str.size(), "application/json");
  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      CTL_INF("Abort model pull successfully: " << task_id);
      return true;
    } else {
      auto root = json_helper::ParseJsonString(res->body);
      CLI_LOG(root["message"].asString());
      return false;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return false;
  }
}
};  // namespace commands
