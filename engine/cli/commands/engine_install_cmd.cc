#include "engine_install_cmd.h"
#include <future>
#include "server_start_cmd.h"
#include "utils/download_progress.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"

namespace commands {
bool EngineInstallCmd::Exec(const std::string& engine,
                            const std::string& version,
                            const std::string& src) {
  // Handle local install, if fails, fallback to remote install
  if (!src.empty()) {
    auto res = engine_service_.UnzipEngine(engine, version, src);
    if (res.has_error()) {
      CLI_LOG(res.error());
      return false;
    }
    if (res.value()) {
      CLI_LOG("Engine " << engine << " installed successfully!");
      return true;
    }
  }

  // Start server if server is not started yet
  if (!commands::IsServerAlive(host_, port_)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host_, port_)) {
      return false;
    }
  }

  DownloadProgress dp;
  dp.Connect(host_, port_);
  // engine can be small, so need to start ws first
  auto dp_res = std::async(std::launch::deferred, [&dp] {
    return dp.Handle(DownloadType::Engine);
  });
  CLI_LOG("Validating download items, please wait..")

  httplib::Client cli(host_ + ":" + std::to_string(port_));
  Json::Value json_data;
  json_data["version"] = version.empty() ? "latest" : version;
  auto data_str = json_data.toStyledString();
  cli.set_read_timeout(std::chrono::seconds(60));
  auto res = cli.Post("/v1/engines/install/" + engine, httplib::Headers(),
                      data_str.data(), data_str.size(), "application/json");

  if (res) {
    if (res->status != httplib::StatusCode::OK_200) {
      auto root = json_helper::ParseJsonString(res->body);
      CLI_LOG(root["message"].asString());
      dp.ForceStop();
      return false;
    } else {
      CLI_LOG("Start downloading..");
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    dp.ForceStop();
    return false;
  }

  if (!dp_res.get())
    return false;

  bool check_cuda_download = !system_info_utils::GetCudaVersion().empty();
  if (check_cuda_download) {
    if (!dp.Handle(DownloadType::CudaToolkit))
      return false;
  }

  CLI_LOG("Engine " << engine << " downloaded successfully!")
  return true;
}
};  // namespace commands
