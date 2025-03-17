#include "engine_install_cmd.h"
#include <future>
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/download_progress.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

namespace commands {

// NOTE: should have a single source of truth between CLI and server
static bool NeedCudaDownload(const std::string& engine) {
  return !system_info_utils::GetDriverAndCudaVersion().second.empty() &&
         engine == kLlamaRepo;
}

bool EngineInstallCmd::Exec(const std::string& engine,
                            const std::string& version,
                            const std::string& src) {
  // Handle local install, if fails, fallback to remote install
  if (!src.empty()) {
    auto res = engine_service_->UnzipEngine(engine, version, src);
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
    bool need_cuda_download = NeedCudaDownload(engine);
    // engine can be small, so need to start ws first
    auto dp_res = std::async(std::launch::deferred, [&dp, need_cuda_download, engine] {
      // if (need_cuda_download) {
      //   return dp.Handle({DownloadType::Engine, DownloadType::CudaToolkit});
      // } else {
      //   return dp.Handle({DownloadType::Engine});
      // }
      if (engine == kLlamaRepo)
        return dp.Handle({DownloadType::Engine, DownloadType::CudaToolkit});
      else
        return dp.Handle({});
    });

    auto releases_url = url_parser::Url{
        .protocol = "http",
        .host = host_ + ":" + std::to_string(port_),
        .pathParams = {"v1", "engines", engine, "releases"},
    };
    auto releases_result = curl_utils::SimpleGetJson(releases_url.ToFullPath());
    if (releases_result.has_error()) {
      CTL_ERR(releases_result.error());
      return false;
    }
    std::vector<std::string> version_selections;
    for (const auto& release_version : releases_result.value()) {
      version_selections.push_back(release_version["name"].asString());
    }

    auto selected_release =
        cli_selection_utils::PrintSelection(version_selections);
    if (selected_release == std::nullopt) {
      CTL_ERR("Invalid version selection");
      return false;
    }
    std::cout << "Selected version: " << selected_release.value() << std::endl;

    auto variant_url = url_parser::Url{
        .protocol = "http",
        .host = host_ + ":" + std::to_string(port_),
        .pathParams =
            {
                "v1",
                "engines",
                engine,
                "releases",
                selected_release.value(),
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
      if (string_utils::StringContainsIgnoreCase(v_name, hw_inf_.sys_inf->os) &&
          string_utils::StringContainsIgnoreCase(v_name,
                                                 hw_inf_.sys_inf->arch)) {
        variant_selections.push_back(variant["name"].asString());
      }
    }
    if (variant_selections.empty()) {
      CTL_ERR("No suitable variant found for " << hw_inf_.sys_inf->os << " "
                                               << hw_inf_.sys_inf->arch);
      return false;
    }

    std::optional<std::string> selected_variant = std::nullopt;
    if (variant_selections.size() == 1) {
      selected_variant = variant_selections[0];
    } else {
      selected_variant =
          cli_selection_utils::PrintSelection(variant_selections);
    }
    if (selected_variant == std::nullopt) {
      CTL_ERR("Invalid variant selection");
      return false;
    }
    std::cout << "Selected " << selected_variant.value() << " - "
              << selected_release.value() << std::endl;

    auto install_url = url_parser::Url{
        .protocol = "http",
        .host = host_ + ":" + std::to_string(port_),
        .pathParams =
            {
                "v1",
                "engines",
                engine,
                "install",
            },
    };
    Json::Value body;
    body["version"] = selected_release.value();
    body["variant"] = selected_variant.value();

    auto response = curl_utils::SimplePostJson(install_url.ToFullPath(),
                                               body.toStyledString());
    if (response.has_error()) {
      CTL_ERR(response.error());
      return false;
    }

    if (!dp_res.get())
      return false;

    CLI_LOG("Engine " << engine << " downloaded successfully!")
    return true;
  }

  // default
  DownloadProgress dp;
  dp.Connect(host_, port_);
  bool need_cuda_download = NeedCudaDownload(engine);
  // engine can be small, so need to start ws first
  auto dp_res = std::async(std::launch::deferred, [&dp, need_cuda_download, engine] {
    // if (need_cuda_download) {
    //   return dp.Handle({DownloadType::Engine, DownloadType::CudaToolkit});
    // } else {
    //   return dp.Handle({DownloadType::Engine});
    // }
    if (engine == kLlamaRepo)
      return dp.Handle({DownloadType::Engine, DownloadType::CudaToolkit});
    else
      return dp.Handle({});
  });

  auto install_url = url_parser::Url{
      .protocol = "http",
      .host = host_ + ":" + std::to_string(port_),
      .pathParams =
          {
              "v1",
              "engines",
              engine,
              "install",
          },
  };

  Json::Value body;
  if (!version.empty()) {
    body["version"] = version;
  }

  auto response = curl_utils::SimplePostJson(install_url.ToFullPath(),
                                             body.toStyledString());
  if (response.has_error()) {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(response.error(), root)) {
      CLI_LOG(response.error());
      return false;
    }
    CLI_LOG(root["message"].asString());
    return false;
  }

  if (!dp_res.get())
    return false;

  CLI_LOG("Engine " << engine << " downloaded successfully!")
  return true;
}
};  // namespace commands
