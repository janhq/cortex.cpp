#include "engine_init_cmd.h"
#include <utility>
#include "services/download_service.h"
#include "trantor/utils/Logger.h"
// clang-format off
#include "utils/cortexso_parser.h" 
#include "utils/archive_utils.h"   
#include "utils/system_info_utils.h"
// clang-format on
#include "utils/cuda_toolkit_utils.h"
#include "utils/engine_matcher_utils.h"
#include "utils/file_manager_utils.h"

namespace commands {

EngineInitCmd::EngineInitCmd(std::string engineName, std::string version)
    : engineName_(std::move(engineName)), version_(std::move(version)) {}

bool EngineInitCmd::Exec() const {
  auto system_info = system_info_utils::GetSystemInfo();
  constexpr auto gitHubHost = "https://api.github.com";
  // std::string version = version_.empty() ? "latest" : version_;
  std::ostringstream engineReleasePath;
  engineReleasePath << "/repos/janhq/" << engineName_ << "/releases";
  if (version_ == "latest") {
    engineReleasePath << "/latest";
  }
  CTL_INF("Engine release path: " << gitHubHost << engineReleasePath.str());
  using namespace nlohmann;

  httplib::Client cli(gitHubHost);
  if (auto res = cli.Get(engineReleasePath.str())) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto jsonResponse = json::parse(res->body);

        nlohmann::json json_data;
        if (version_ == "latest") {
          json_data = jsonResponse;
        } else {
          for (auto& jr : jsonResponse) {
            // Get the latest or match version
            if (auto tag = jr["tag_name"].get<std::string>(); tag == version_) {
              json_data = jr;
              break;
            }
          }
        }

        if (json_data.empty()) {
          CLI_LOG("Version not found: " << version_);
          return false;
        }

        auto assets = json_data["assets"];
        auto os_arch{system_info.os + "-" + system_info.arch};

        std::vector<std::string> variants;
        for (auto& asset : assets) {
          auto asset_name = asset["name"].get<std::string>();
          variants.push_back(asset_name);
        }

        auto cuda_driver_version = system_info_utils::GetCudaVersion();
        CTL_INF("engineName_: " << engineName_);
        CTL_INF("CUDA version: " << cuda_driver_version);
        std::string matched_variant = "";

        if (engineName_ == "cortex.tensorrt-llm") {
          matched_variant = engine_matcher_utils::ValidateTensorrtLlm(
              variants, system_info.os, cuda_driver_version);
        } else if (engineName_ == "cortex.onnx") {
          matched_variant = engine_matcher_utils::ValidateOnnx(
              variants, system_info.os, system_info.arch);
        } else if (engineName_ == "cortex.llamacpp") {
          cortex::cpuid::CpuInfo cpu_info;
          auto suitable_avx =
              engine_matcher_utils::GetSuitableAvxVariant(cpu_info);
          matched_variant = engine_matcher_utils::Validate(
              variants, system_info.os, system_info.arch, suitable_avx,
              cuda_driver_version);
        }
        CTL_INF("Matched variant: " << matched_variant);
        if (matched_variant.empty()) {
          CTL_ERR("No variant found for " << os_arch);
          return false;
        }

        for (auto& asset : assets) {
          auto assetName = asset["name"].get<std::string>();
          if (assetName == matched_variant) {
            auto download_url =
                asset["browser_download_url"].get<std::string>();
            auto file_name = asset["name"].get<std::string>();
            CTL_INF("Download url: " << download_url);

            std::filesystem::path engine_folder_path =
                file_manager_utils::GetContainerFolderPath(
                    file_manager_utils::DownloadTypeToString(
                        DownloadType::Engine)) /
                engineName_;

            if (!std::filesystem::exists(engine_folder_path)) {
              CTL_INF("Creating " << engine_folder_path.string());
              std::filesystem::create_directories(engine_folder_path);
            }

            CTL_INF("Engine folder path: " << engine_folder_path.string()
                                           << "\n");
            auto local_path = engine_folder_path / file_name;
            auto downloadTask{DownloadTask{.id = engineName_,
                                           .type = DownloadType::Engine,
                                           .items = {DownloadItem{
                                               .id = engineName_,
                                               .downloadUrl = download_url,
                                               .localPath = local_path,
                                           }}}};

            DownloadService download_service;
            download_service.AddDownloadTask(
                downloadTask, [](const DownloadTask& finishedTask) {
                  // try to unzip the downloaded file
                  CTL_INF("Engine zip path: "
                          << finishedTask.items[0].localPath.string());

                  std::filesystem::path extract_path =
                      finishedTask.items[0]
                          .localPath.parent_path()
                          .parent_path();

                  archive_utils::ExtractArchive(
                      finishedTask.items[0].localPath.string(),
                      extract_path.string());

                  // remove the downloaded file
                  try {
                    std::filesystem::remove(finishedTask.items[0].localPath);
                  } catch (const std::exception& e) {
                    CTL_WRN("Could not delete file: " << e.what());
                  }
                  CTL_INF("Finished!");
                });
            if (system_info.os == "mac" || engineName_ == "cortex.onnx") {
              // mac and onnx engine does not require cuda toolkit
              return true;
            }

            if (cuda_driver_version.empty()) {
              CTL_WRN("No cuda driver, continue with CPU");
              return true;
            }

            // download cuda toolkit
            const std::string jan_host = "https://catalog.jan.ai";
            const std::string cuda_toolkit_file_name = "cuda.tar.gz";
            const std::string download_id = "cuda";

            // TODO: we don't have API to retrieve list of cuda toolkit dependencies atm because we hosting it at jan
            //  will have better logic after https://github.com/janhq/cortex/issues/1046 finished
            // for now, assume that we have only 11.7 and 12.4
            auto suitable_toolkit_version = "";
            if (engineName_ == "cortex.tensorrt-llm") {
              // for tensorrt-llm, we need to download cuda toolkit v12.4
              suitable_toolkit_version = "12.4";
            } else {
              // llamacpp
              auto cuda_driver_semver =
                  semantic_version_utils::SplitVersion(cuda_driver_version);
              if (cuda_driver_semver.major == 11) {
                suitable_toolkit_version = "11.7";
              } else if (cuda_driver_semver.major == 12) {
                suitable_toolkit_version = "12.4";
              }
            }

            // compare cuda driver version with cuda toolkit version
            // cuda driver version should be greater than toolkit version to ensure compatibility
            if (semantic_version_utils::CompareSemanticVersion(
                    cuda_driver_version, suitable_toolkit_version) < 0) {
              CTL_ERR("Your Cuda driver version "
                      << cuda_driver_version
                      << " is not compatible with cuda toolkit version "
                      << suitable_toolkit_version);
              return false;
            }

            std::ostringstream cuda_toolkit_url;
            cuda_toolkit_url << jan_host << "/" << "dist/cuda-dependencies/"
                             << cuda_driver_version << "/" << system_info.os
                             << "/" << cuda_toolkit_file_name;

            LOG_DEBUG << "Cuda toolkit download url: "
                      << cuda_toolkit_url.str();
            auto cuda_toolkit_local_path =
                file_manager_utils::GetContainerFolderPath(
                    file_manager_utils::DownloadTypeToString(
                        DownloadType::CudaToolkit)) /
                cuda_toolkit_file_name;
            LOG_DEBUG << "Download to: " << cuda_toolkit_local_path.string();
            auto downloadCudaToolkitTask{DownloadTask{
                .id = download_id,
                .type = DownloadType::CudaToolkit,
                .items = {DownloadItem{.id = download_id,
                                       .downloadUrl = cuda_toolkit_url.str(),
                                       .localPath = cuda_toolkit_local_path}},
            }};

            download_service.AddDownloadTask(
                downloadCudaToolkitTask, [&](const DownloadTask& finishedTask) {
                  auto engine_path =
                      file_manager_utils::GetEnginesContainerPath() /
                      engineName_;
                  archive_utils::ExtractArchive(
                      finishedTask.items[0].localPath.string(),
                      engine_path.string());

                  try {
                    std::filesystem::remove(finishedTask.items[0].localPath);
                  } catch (std::exception& e) {
                    CTL_ERR("Error removing downloaded file: " << e.what());
                  }
                });
            return true;
          }
        }
      } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
      }
    } else {
      CTL_ERR("HTTP error: " << res->status);
      return false;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return false;
  }
  return true;
}
};  // namespace commands
