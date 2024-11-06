#include "engine_install_cmd.h"
#include <future>
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/download_progress.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

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

  if (show_menu_) {
    DownloadProgress dp;
    dp.Connect(host_, port_);
    // engine can be small, so need to start ws first
    auto dp_res = std::async(std::launch::deferred, [&dp, &engine] {
      return dp.Handle(DownloadType::Engine);
    });

    auto versions_url = url_parser::Url{
        .protocol = "http",
        .host = host_ + ":" + std::to_string(port_),
        .pathParams = {"v1", "engines", engine, "versions"},
    };
    auto versions_result = curl_utils::SimpleGetJson(versions_url.ToFullPath());
    if (versions_result.has_error()) {
      CTL_ERR(versions_result.error());
      return false;
    }
    std::vector<std::string> version_selections;
    for (const auto& release_version : versions_result.value()) {
      version_selections.push_back(release_version["name"].asString());
    }

    auto selected_version =
        cli_selection_utils::PrintSelection(version_selections);
    if (selected_version == std::nullopt) {
      CTL_ERR("Invalid version selection");
      return false;
    }
    std::cout << "Selected version: " << selected_version.value() << std::endl;

    auto variant_url = url_parser::Url{
        .protocol = "http",
        .host = host_ + ":" + std::to_string(port_),
        .pathParams =
            {
                "v1",
                "engines",
                engine,
                "versions",
                selected_version.value(),
            },
    };
    auto variant_result = curl_utils::SimpleGetJson(variant_url.ToFullPath());
    if (variant_result.has_error()) {
      CTL_ERR(variant_result.error());
      return false;
    }

    std::vector<std::string> variant_selections;
    for (const auto& variant : variant_result.value()) {
      auto v_name = variant["name"].asString();
      if (string_utils::StringContainsIgnoreCase(v_name, hw_inf_.sys_inf->os)) {
        variant_selections.push_back(variant["name"].asString());
      }
    }
    auto selected_variant =
        cli_selection_utils::PrintSelection(variant_selections);
    if (selected_variant == std::nullopt) {
      CTL_ERR("Invalid variant selection");
      return false;
    }
    std::cout << "Selected " << selected_variant.value() << " - "
              << selected_version.value() << std::endl;

    auto install_url =
        url_parser::Url{.protocol = "http",
                        .host = host_ + ":" + std::to_string(port_),
                        .pathParams =
                            {
                                "v1",
                                "engines",
                                engine,
                            },
                        .queries = {
                            {"version", selected_version.value()},
                            {"variant", selected_variant.value()},
                        }};

    auto response = curl_utils::SimplePostJson(install_url.ToFullPath());
    if (response.has_error()) {
      CTL_ERR(response.error());
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

  // default
  DownloadProgress dp;
  dp.Connect(host_, port_);
  // engine can be small, so need to start ws first
  auto dp_res = std::async(std::launch::deferred,
                           [&dp] { return dp.Handle(DownloadType::Engine); });

  auto install_url = url_parser::Url{
      .protocol = "http",
      .host = host_ + ":" + std::to_string(port_),
      .pathParams =
          {
              "v1",
              "engines",
              engine,
          },
  };

  if (!version.empty()) {
    install_url.queries = {{"version", version}};
  }

  auto response = curl_utils::SimplePostJson(install_url.ToFullPath());
  if (response.has_error()) {
    // TODO: namh refactor later
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(response.error(), root)) {
      CLI_LOG(response.error());
      return false;
    }
    CLI_LOG(root["message"].asString());
    return false;
  }

  CLI_LOG("Validating download items, please wait..")

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
