#include "model_pull_cmd.h"
#include <csignal>
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/curl_utils.h"
#include "utils/download_progress.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/scope_exit.h"
#include "utils/url_parser.h"
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

  auto model_info_url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"models", "pull", "info"},
      /* .queries = */ {},
  };
  Json::Value j_data;
  j_data["model"] = input;
  auto d_str = j_data.toStyledString();
  auto res = curl_utils::SimplePostJson(model_info_url.ToFullPath(), d_str);

  if (res.has_error()) {
    auto root = json_helper::ParseJsonString(res.error());
    CLI_LOG(root["message"].asString());
    return std::nullopt;
  }

  auto id = res.value()["id"].asString();
  bool is_cortexso = res.value()["modelSource"].asString() == "cortexso";
  auto default_branch = res.value()["defaultBranch"].asString();
  std::vector<std::string> downloaded;
  for (auto const& v : res.value()["downloadedModels"]) {
    downloaded.push_back(v.asString());
  }
  std::vector<std::string> avails;
  for (auto const& v : res.value()["availableModels"]) {
    avails.push_back(v.asString());
  }
  auto download_url = res.value()["downloadUrl"].asString();

  if (downloaded.empty() && avails.empty()) {
    if (res.value()["modelSource"].asString() == "huggingface") {
      model = id;
    } else {
      model_id = id;
      model = download_url;
    }
  } else {
    if (is_cortexso) {
      auto selection = cli_selection_utils::PrintModelSelection(
          downloaded, avails,
          default_branch.empty() ? std::nullopt
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

  CTL_INF("model: " << model << ", model_id: " << model_id);

  Json::Value json_data;
  json_data["model"] = model;
  auto data_str = json_data.toStyledString();

  auto pull_url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "models", "pull"},
      /* .queries = */ {},
  };

  auto pull_result =
      curl_utils::SimplePostJson(pull_url.ToFullPath(), data_str);
  if (pull_result.has_error()) {
    auto root = json_helper::ParseJsonString(pull_result.error());
    CLI_LOG(root["message"].asString());
    return std::nullopt;
  }

  CLI_LOG("Start downloading..")
  DownloadProgress dp;
  bool force_stop = false;

  shutdown_handler = [this, &dp, &host, &port, &model_id, &force_stop](int) {
    force_stop = true;
    AbortModelPull(host, port, model_id);
    dp.ForceStop();
  };

  cortex::utils::ScopeExit se([]() { shutdown_handler = {}; });
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
  if (!dp.Handle({DownloadType::Model}))
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
  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"v1", "models", "pull"},
      /* .queries = */ {},
  };
  auto res = curl_utils::SimpleDeleteJson(url.ToFullPath(), data_str);

  if (res.has_error()) {
    auto root = json_helper::ParseJsonString(res.error());
    CLI_LOG(root["message"].asString());
    return false;
  }
  CTL_INF("Abort model pull successfully: " << task_id);
  return true;
}
};  // namespace commands
