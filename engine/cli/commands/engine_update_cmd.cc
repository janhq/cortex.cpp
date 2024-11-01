#include "engine_update_cmd.h"
#include <future>
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/curl_utils.h"
#include "utils/download_progress.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"
#include "utils/url_parser.h"

namespace commands {
bool EngineUpdateCmd::Exec(const std::string& host, int port,
                           const std::string& engine) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return false;
    }
  }

  DownloadProgress dp;
  dp.Connect(host, port);
  // engine can be small, so need to start ws first
  auto dp_res = std::async(std::launch::deferred, [&dp, &engine] {
    return dp.Handle(DownloadType::Engine);
  });
  CLI_LOG("Validating download items, please wait..")

  auto update_url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "engines", engine, "update"},
  };
  auto update_result = curl_utils::SimplePostJson(update_url.ToFullPath());
  if (update_result.has_error()) {
    CTL_ERR(update_result.error());
    return false;
  }

  if (!dp_res.get())
    return false;

  bool check_cuda_download = !system_info_utils::GetCudaVersion().empty();
  if (check_cuda_download) {
    if (!dp.Handle(DownloadType::CudaToolkit))
      return false;
  }

  CLI_LOG("Engine " << engine << " updated successfully!")
  return true;
}
};  // namespace commands
