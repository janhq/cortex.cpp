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
#if defined(_WIN32) || defined(__linux__)
#include "utils/file_manager_utils.h"
#endif

namespace commands {

EngineInitCmd::EngineInitCmd(std::string engineName, std::string version)
    : engineName_(std::move(engineName)), version_(std::move(version)) {}

bool EngineInitCmd::Exec() const {
  if (engineName_.empty()) {
    CTL_ERR("Engine name is required");
    return false;
  }

  // Check if the architecture and OS are supported
  auto system_info = system_info_utils::GetSystemInfo();
  if (system_info.arch == system_info_utils::kUnsupported ||
      system_info.os == system_info_utils::kUnsupported) {
    CTL_ERR("Unsupported OS or architecture: " << system_info.os << ", "
                                               << system_info.arch);
    return false;
  }
  CTL_INF("OS: " << system_info.os << ", Arch: " << system_info.arch);

  // check if engine is supported
  if (std::find(supportedEngines_.begin(), supportedEngines_.end(),
                engineName_) == supportedEngines_.end()) {
    CTL_ERR("Engine not supported");
    return false;
  }

  constexpr auto gitHubHost = "https://api.github.com";
  std::string version = version_.empty() ? "latest" : version_;
  std::ostringstream engineReleasePath;
  engineReleasePath << "/repos/janhq/" << engineName_ << "/releases/"
                    << version;
  CTL_INF("Engine release path: " << gitHubHost << engineReleasePath.str());
  using namespace nlohmann;

  httplib::Client cli(gitHubHost);
  if (auto res = cli.Get(engineReleasePath.str())) {
    if (res->status == httplib::StatusCode::OK_200) {
      try {
        auto jsonResponse = json::parse(res->body);
        auto assets = jsonResponse["assets"];
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
            std::string host{"https://github.com"};

            auto full_url = asset["browser_download_url"].get<std::string>();
            std::string path = full_url.substr(host.length());

            auto fileName = asset["name"].get<std::string>();
            CTL_INF("URL: " << full_url);

            auto downloadTask = DownloadTask{.id = engineName_,
                                             .type = DownloadType::Engine,
                                             .error = std::nullopt,
                                             .items = {DownloadItem{
                                                 .id = engineName_,
                                                 .host = host,
                                                 .fileName = fileName,
                                                 .type = DownloadType::Engine,
                                                 .path = path,
                                             }}};

            DownloadService download_service;
            download_service.AddDownloadTask(downloadTask, [this](
                                                               const std::string&
                                                                   absolute_path,
                                                               bool unused) {
              // try to unzip the downloaded file
              std::filesystem::path downloadedEnginePath{absolute_path};
              CTL_INF(
                  "Downloaded engine path: " << downloadedEnginePath.string());

              std::filesystem::path extract_path =
                  downloadedEnginePath.parent_path().parent_path();

              archive_utils::ExtractArchive(downloadedEnginePath.string(),
                                            extract_path.string());
#if defined(_WIN32) || defined(__linux__)
              // FIXME: hacky try to copy the file. Remove this when we are able to set the library path
              auto engine_path = extract_path / engineName_;
              LOG_INFO << "Source path: " << engine_path.string();
              auto executable_path =
                  file_manager_utils::GetExecutableFolderContainerPath();
              for (const auto& entry :
                   std::filesystem::recursive_directory_iterator(engine_path)) {
                if (entry.is_regular_file() &&
                    entry.path().extension() != ".gz") {
                  std::filesystem::path relative_path =
                      std::filesystem::relative(entry.path(), engine_path);
                  std::filesystem::path destFile =
                      executable_path / relative_path;

                  std::filesystem::create_directories(destFile.parent_path());
                  std::filesystem::copy_file(
                      entry.path(), destFile,
                      std::filesystem::copy_options::overwrite_existing);

                  std::cout << "Copied: " << entry.path().filename().string()
                            << " to " << destFile.string() << std::endl;
                }
              }
              std::cout << "DLL copying completed successfully." << std::endl;
#endif

              // remove the downloaded file
              // TODO(any) Could not delete file on Windows because it is currently hold by httplib(?)
              // Not sure about other platforms
              try {
                std::filesystem::remove(absolute_path);
              } catch (const std::exception& e) {
                CTL_WRN("Could not delete file: " << e.what());
              }
              CTL_INF("Finished!");
            });
            if (system_info.os == "mac" || engineName_ == "cortex.onnx") {
              // mac and onnx engine does not require cuda toolkit
              return true;
            }

            // download cuda toolkit
            const std::string jan_host = "https://catalog.jan.ai";
            const std::string cuda_toolkit_file_name = "cuda.tar.gz";
            const std::string download_id = "cuda";

            // TODO: we don't have API to retrieve list of cuda toolkit dependencies atm because we hosting it at jan
            // will have better logic after https://github.com/janhq/cortex/issues/1046 finished
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

            std::ostringstream cuda_toolkit_path;
            cuda_toolkit_path << "dist/cuda-dependencies/"
                              << cuda_driver_version << "/" << system_info.os
                              << "/" << cuda_toolkit_file_name;

            LOG_DEBUG << "Cuda toolkit download url: " << jan_host
                      << cuda_toolkit_path.str();

            auto downloadCudaToolkitTask = DownloadTask{
                .id = download_id,
                .type = DownloadType::CudaToolkit,
                .error = std::nullopt,
                .items = {DownloadItem{
                    .id = download_id,
                    .host = jan_host,
                    .fileName = cuda_toolkit_file_name,
                    .type = DownloadType::CudaToolkit,
                    .path = cuda_toolkit_path.str(),
                }},
            };

            download_service.AddDownloadTask(
                downloadCudaToolkitTask,
                [](const std::string& absolute_path, bool unused) {
                  LOG_DEBUG << "Downloaded cuda path: " << absolute_path;
                  // try to unzip the downloaded file
                  std::filesystem::path downloaded_path{absolute_path};

                  archive_utils::ExtractArchive(
                      absolute_path,
                      downloaded_path.parent_path().parent_path().string());

                  try {
                    std::filesystem::remove(absolute_path);
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
