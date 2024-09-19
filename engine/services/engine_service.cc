#include "engine_service.h"
#include <httplib.h>
#include <stdexcept>
#include "algorithm"
#include "utils/archive_utils.h"
#include "utils/engine_matcher_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/json.hpp"
#include "utils/semantic_version_utils.h"
#include "utils/system_info_utils.h"
#include "utils/url_parser.h"

using json = nlohmann::json;

std::optional<EngineInfo> EngineService::GetEngineInfo(
    const std::string& engine) const {
  // if engine is not found in kSupportEngine, throw runtime error
  if (std::find(kSupportEngines.begin(), kSupportEngines.end(), engine) ==
      kSupportEngines.end()) {
    return std::nullopt;
  }

  auto engine_status_list = GetEngineInfoList();

  return *std::find_if(
      engine_status_list.begin(), engine_status_list.end(),
      [&engine](const EngineInfo& e) { return e.name == engine; });
}

std::vector<EngineInfo> EngineService::GetEngineInfoList() const {
  auto ecp = file_manager_utils::GetEnginesContainerPath();

  std::string onnx_status{kIncompatible};
  std::string llamacpp_status =
      std::filesystem::exists(ecp / "cortex.llamacpp") ? kReady : kNotInstalled;
  std::string tensorrt_status{kIncompatible};

#ifdef _WIN32
  onnx_status =
      std::filesystem::exists(ecp / "cortex.onnx") ? kReady : kNotInstalled;
  tensorrt_status = std::filesystem::exists(ecp / "cortex.tensorrt-llm")
                        ? kReady
                        : kNotInstalled;
#elif defined(__linux__)
  tensorrt_status = std::filesystem::exists(ecp / "cortex.tensorrt-llm")
                        ? kReady
                        : kNotInstalled;
#endif
  std::vector<EngineInfo> engines = {
      {.name = "cortex.onnx",
       .description = "This extension enables chat completion API calls using "
                      "the Onnx engine",
       .format = "ONNX",
       .version = "0.0.1",
       .product_name = "ONNXRuntime",
       .status = onnx_status},
      {.name = "cortex.llamacpp",
       .description = "This extension enables chat completion API calls using "
                      "the LlamaCPP engine",
       .format = "GGUF",
       .version = "0.0.1",
       .product_name = "llama.cpp",
       .status = llamacpp_status},
      {.name = "cortex.tensorrt-llm",
       .description = "This extension enables chat completion API calls using "
                      "the TensorrtLLM engine",
       .format = "TensorRT Engines",
       .version = "0.0.1",
       .product_name = "TensorRT-LLM",
       .status = tensorrt_status},
  };

  return engines;
}

void EngineService::InstallEngine(const std::string& engine,
                                  const std::string& version) {
  auto system_info = system_info_utils::GetSystemInfo();
  auto get_params = [&engine, &version]() -> std::vector<std::string> {
    if (version == "latest") {
      return {"repos", "janhq", engine, "releases", version};
    } else {
      return {"repos", "janhq", engine, "releases"};
    }
  };

  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = "api.github.com",
      .pathParams = get_params(),
  };

  httplib::Client cli(url_obj.GetProtocolAndHost());
  if (auto res = cli.Get(url_obj.GetPathAndQuery());
      res->status == httplib::StatusCode::OK_200) {
    auto body = json::parse(res->body);
    auto get_data =
        [&version](const nlohmann::json& json_data) -> nlohmann::json {
      for (auto& jr : json_data) {
        // Get the latest or match version
        if (auto tag = jr["tag_name"].get<std::string>(); tag == version) {
          return jr;
        }
      }
      return nlohmann::json();
    };

    if (version != "latest") {
      body = get_data(body);
    }
    if (body.empty()) {
      throw std::runtime_error("No release found for " + version);
    }

    auto assets = body["assets"];
    auto os_arch{system_info.os + "-" + system_info.arch};

    std::vector<std::string> variants;
    for (auto& asset : assets) {
      auto asset_name = asset["name"].get<std::string>();
      variants.push_back(asset_name);
    }

    auto cuda_driver_version = system_info_utils::GetCudaVersion();
    CTL_INF("engine: " << engine);
    CTL_INF("CUDA version: " << cuda_driver_version);
    std::string matched_variant = "";

    if (engine == "cortex.tensorrt-llm") {
      matched_variant = engine_matcher_utils::ValidateTensorrtLlm(
          variants, system_info.os, cuda_driver_version);
    } else if (engine == "cortex.onnx") {
      matched_variant = engine_matcher_utils::ValidateOnnx(
          variants, system_info.os, system_info.arch);
    } else if (engine == "cortex.llamacpp") {
      cortex::cpuid::CpuInfo cpu_info;
      auto suitable_avx = engine_matcher_utils::GetSuitableAvxVariant(cpu_info);
      matched_variant = engine_matcher_utils::Validate(
          variants, system_info.os, system_info.arch, suitable_avx,
          cuda_driver_version);
    }
    CTL_INF("Matched variant: " << matched_variant);
    if (matched_variant.empty()) {
      CTL_ERR("No variant found for " << os_arch);
      throw std::runtime_error("No variant found for " + os_arch);
    }

    for (auto& asset : assets) {
      auto assetName = asset["name"].get<std::string>();
      if (assetName == matched_variant) {
        auto download_url = asset["browser_download_url"].get<std::string>();
        auto file_name = asset["name"].get<std::string>();
        CTL_INF("Download url: " << download_url);

        std::filesystem::path engine_folder_path =
            file_manager_utils::GetContainerFolderPath(
                file_manager_utils::DownloadTypeToString(
                    DownloadType::Engine)) /
            engine;

        if (!std::filesystem::exists(engine_folder_path)) {
          CTL_INF("Creating " << engine_folder_path.string());
          std::filesystem::create_directories(engine_folder_path);
        }

        CTL_INF("Engine folder path: " << engine_folder_path.string() << "\n");
        auto local_path = engine_folder_path / file_name;
        auto downloadTask{DownloadTask{.id = engine,
                                       .type = DownloadType::Engine,
                                       .items = {DownloadItem{
                                           .id = engine,
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
                  finishedTask.items[0].localPath.parent_path().parent_path();

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
        if (system_info.os == "mac" || engine == "cortex.onnx") {
          // mac and onnx engine does not require cuda toolkit
          return;
        }

        if (cuda_driver_version.empty()) {
          CTL_WRN("No cuda driver, continue with CPU");
          return;
        }

        // download cuda toolkit
        const std::string jan_host = "https://catalog.jan.ai";
        const std::string cuda_toolkit_file_name = "cuda.tar.gz";
        const std::string download_id = "cuda";

        // TODO: we don't have API to retrieve list of cuda toolkit dependencies atm because we hosting it at jan
        //  will have better logic after https://github.com/janhq/cortex/issues/1046 finished
        // for now, assume that we have only 11.7 and 12.4
        auto suitable_toolkit_version = "";
        if (engine == "cortex.tensorrt-llm") {
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
          throw std::runtime_error(
              "Cuda driver is not compatible with cuda toolkit");
        }

        std::ostringstream cuda_toolkit_url;
        cuda_toolkit_url << jan_host << "/" << "dist/cuda-dependencies/"
                         << cuda_driver_version << "/" << system_info.os << "/"
                         << cuda_toolkit_file_name;

        LOG_DEBUG << "Cuda toolkit download url: " << cuda_toolkit_url.str();
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
                  file_manager_utils::GetEnginesContainerPath() / engine;
              archive_utils::ExtractArchive(
                  finishedTask.items[0].localPath.string(),
                  engine_path.string());

              try {
                std::filesystem::remove(finishedTask.items[0].localPath);
              } catch (std::exception& e) {
                CTL_ERR("Error removing downloaded file: " << e.what());
              }
            });
        return;
      }
    }
  } else {
    throw std::runtime_error("Failed to fetch engine release: " + engine);
  }
}

void EngineService::InstallLocalEngine(const std::string& engine,
                                       const std::string& path) {
  auto system_info = system_info_utils::GetSystemInfo();
  std::string cuda_variant = "";
  // Get CPU, GPU info
  auto cuda_driver_version = system_info_utils::GetCudaVersion();
  CTL_INF("engine: " << engine);
  CTL_INF("CUDA version: " << cuda_driver_version);
  std::string matched_variant = "";
  if (engine == "cortex.tensorrt-llm") {
    matched_variant = engine_matcher_utils::ValidateTensorrtLlm(
        variants, system_info.os, cuda_driver_version);
  } else if (engine == "cortex.onnx") {
    matched_variant = engine_matcher_utils::ValidateOnnx(
        variants, system_info.os, system_info.arch);
  } else if (engine == "cortex.llamacpp") {
    cortex::cpuid::CpuInfo cpu_info;
    auto suitable_avx = engine_matcher_utils::GetSuitableAvxVariant(cpu_info);
    matched_variant = engine_matcher_utils::Validate(
        variants, system_info.os, system_info.arch, suitable_avx,
        cuda_driver_version);
  }
  CTL_INF("Matched variant: " << matched_variant);
  if (matched_variant.empty()) {
    CTL_ERR("No variant found for " << os_arch);
    throw std::runtime_error("No variant found for " + os_arch);
  }

  if (engine == "cortex.tensorrt-llm") {
    // for tensorrt-llm, we need to download cuda toolkit v12.4
    cuda_variant = "cuda-12.4.tar.gz";
  } else {
    // llamacpp
    auto cuda_driver_semver =
        semantic_version_utils::SplitVersion(cuda_driver_version);
    cuda_variant = "cuda-" + std::to_string(cuda_driver_semver.major) + "." +
                   std::to_string(cuda_driver_semver.minor) + "tar.gz";
  }

  CTL_INF("Matched cuda variant: " << cuda_variant);
  if (cuda_variant.empty()) {
    CTL_ERR("No cuda variant found for " << engine);
    throw std::runtime_error("No cuda variant found for " + engine);
  }

  // Check if folder exists
  bool found_engine = false;
  bool found_cuda = false;
  if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
    for (const auto& entry : std::filesystem::directory_iterator(models_path)) {
      if (entry.is_regular_file() && entry.path().extension() == ".tar.gz") {
        // Check if match engine binary
        if (std::string filename = entry.path().stem().string();
            filename == matched_variant) {
          found_engine = true;
          // extract binary
          auto engine_path =
              file_manager_utils::GetEnginesContainerPath() / engine;
          archive_utils::ExtractArchive(path + "/" + filename,
                                        engine_path.string());

        } else if (std::string cf = entry.path().stem().string();
                   cf == cuda_variant) {
          found_cuda = true;
          // extract binary
          auto engine_path =
              file_manager_utils::GetEnginesContainerPath() / engine;
          archive_utils::ExtractArchive(path + "/" + cf,
                                        engine_path.string());
        }
      }
    }
  } else {
    // Folder does not exist, throw exception
  }

  // Not match, download from remote
  if (!found_engine) {}

  // Not match any cuda binary, download from remote
  if (!found_cuda) {}
}

void EngineService::UninstallEngine(const std::string& engine) {
  // TODO: Unload the model which is currently running on engine_

  // TODO: Unload engine if is loaded

  auto ecp = file_manager_utils::GetEnginesContainerPath();
  auto engine_path = ecp / engine;

  if (!std::filesystem::exists(engine_path)) {
    throw std::runtime_error("Engine " + engine + " is not installed!");
  }

  try {
    std::filesystem::remove_all(engine_path);
    CTL_INF("Engine " << engine << " uninstalled successfully!");
  } catch (const std::exception& e) {
    CTL_ERR("Failed to uninstall engine " << engine << ": " << e.what());
    throw;
  }
}
