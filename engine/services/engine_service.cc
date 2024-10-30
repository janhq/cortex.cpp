#include "engine_service.h"
#include <cstdlib>
#include <filesystem>
#include <optional>
#include "algorithm"
#include "utils/archive_utils.h"
#include "utils/engine_constants.h"
#include "utils/engine_matcher_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/github_release_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/semantic_version_utils.h"
#include "utils/system_info_utils.h"
#include "utils/url_parser.h"

namespace {
std::string GetSuitableCudaVersion(const std::string& engine,
                                   const std::string& cuda_driver_version) {
  auto suitable_toolkit_version = "";
  if (engine == kTrtLlmRepo || engine == kTrtLlmEngine) {
    // for tensorrt-llm, we need to download cuda toolkit v12.4
    suitable_toolkit_version = "12.4";
  } else {
    // llamacpp
    auto cuda_driver_semver =
        semantic_version_utils::SplitVersion(cuda_driver_version);
    if (cuda_driver_semver.major == 11) {
      suitable_toolkit_version = "11.7";
    } else if (cuda_driver_semver.major == 12) {
      suitable_toolkit_version = "12.0";
    }
  }
  return suitable_toolkit_version;
}

// Need to change this after we rename repositories
std::string NormalizeEngine(const std::string& engine) {
  if (engine == kLlamaEngine) {
    return kLlamaRepo;
  } else if (engine == kOnnxEngine) {
    return kOnnxRepo;
  } else if (engine == kTrtLlmEngine) {
    return kTrtLlmRepo;
  }
  return engine;
};

std::string Repo2Engine(const std::string& r) {
  if (r == kLlamaRepo) {
    return kLlamaEngine;
  } else if (r == kOnnxRepo) {
    return kOnnxEngine;
  } else if (r == kTrtLlmRepo) {
    return kTrtLlmEngine;
  }
  return r;
};

std::string GetEnginePath(std::string_view e) {
  if (e == kLlamaRepo) {
    return kLlamaLibPath;
  } else if (e == kOnnxRepo) {
    return kOnnxLibPath;
  } else if (e == kTrtLlmRepo) {
    return kTensorrtLlmPath;
  }
  return kLlamaLibPath;
};
}  // namespace

cpp::result<void, std::string> EngineService::InstallEngineAsyncV2(
    const std::string& engine, const std::string& version,
    const std::string& variant_name) {
  auto ne = NormalizeEngine(engine);
  CTL_INF("InstallEngineAsyncV2: " << ne << ", " << version << ", "
                                   << variant_name);

  auto result = DownloadEngineV2(ne, version, variant_name, true /*async*/);
  if (result.has_error()) {
    return result;
  }
  auto cuda_res = DownloadCuda(ne, true /*async*/);
  if (cuda_res.has_error()) {
    return cpp::fail(cuda_res.error());
  }
  return {};
}

cpp::result<bool, std::string> EngineService::InstallEngineAsync(
    const std::string& engine, const std::string& version,
    const std::string& src) {
  // Although this function is called async, only download tasks are performed async
  auto ne = NormalizeEngine(engine);
  if (!src.empty()) {
    auto res = UnzipEngine(ne, version, src);
    // If has error or engine is installed successfully
    if (res.has_error() || res.value()) {
      return res;
    }
  }
  auto result = DownloadEngine(ne, version, true /*async*/);
  if (result.has_error()) {
    return result;
  }
  return DownloadCuda(ne, true /*async*/);
}

cpp::result<bool, std::string> EngineService::UnzipEngine(
    const std::string& engine, const std::string& version,
    const std::string& path) {
  bool found_cuda = false;

  CTL_INF("engine: " << engine);
  CTL_INF("CUDA version: " << hw_inf_.cuda_driver_version);
  std::string cuda_variant = "cuda-";
  auto cuda_github =
      GetSuitableCudaVersion(engine, hw_inf_.cuda_driver_version);
  // Github release cuda example: cuda-12-0-windows-amd64.tar.gz
  std::replace(cuda_github.begin(), cuda_github.end(), '.', '-');
  cuda_variant += cuda_github + "-" + hw_inf_.sys_inf->os + "-" +
                  hw_inf_.sys_inf->arch + ".tar.gz";
  CTL_INF("cuda_variant: " << cuda_variant);

  std::vector<std::string> variants;
  // Loop through all files in the directory
  // 1. Push all engine variants to a list
  // 2. If cuda version is matched, extract it
  if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      CTL_INF("file path: " << entry.path().string());
      if (entry.is_regular_file() && (entry.path().extension() == ".tar.gz" ||
                                      entry.path().extension() == ".gz")) {
        CTL_INF("file name: " << entry.path().filename().string());
        variants.push_back(entry.path().filename().string());
        if (std::string cf = entry.path().filename().string();
            cf == cuda_variant) {
          CTL_INF("Found cuda variant, extract it");
          found_cuda = true;
          // extract binary
          auto engine_path =
              file_manager_utils::GetEnginesContainerPath() / engine;
          archive_utils::ExtractArchive(path + "/" + cf, engine_path.string());
        }
      }
    }
  } else {
    // Folder does not exist
    return cpp::fail("Folder does not exist: " + path);
  }

  auto matched_variant = GetMatchedVariant(engine, variants);
  CTL_INF("Matched variant: " << matched_variant);
  if (!found_cuda || matched_variant.empty()) {
    return false;
  }

  if (matched_variant.empty()) {
    CTL_INF("No variant found for " << hw_inf_.sys_inf->os << "-"
                                    << hw_inf_.sys_inf->arch
                                    << ", will get engine from remote");
    // Go with the remote flow
  } else {
    auto engine_path = file_manager_utils::GetEnginesContainerPath();
    archive_utils::ExtractArchive(path + "/" + matched_variant,
                                  engine_path.string());
  }

  return true;
}

cpp::result<bool, std::string> EngineService::UninstallEngineVariant(
    const std::string& engine, const std::string& variant,
    const std::string& version) {
  auto ne = NormalizeEngine(engine);
  auto engine_path =
      file_manager_utils::GetEnginesContainerPath() / ne / variant / version;
  if (!std::filesystem::exists(engine_path)) {
    return cpp::fail("Engine " + ne + " is not installed!");
  }

  try {
    std::filesystem::remove_all(engine_path);
    CTL_INF("Engine " << ne << " uninstalled successfully!");
    return true;
  } catch (const std::exception& e) {
    CTL_ERR("Failed to uninstall engine " << ne << ": " << e.what());
    return cpp::fail("Failed to uninstall engine " + ne + ": " + e.what());
  }
}

cpp::result<void, std::string> EngineService::DownloadEngineV2(
    const std::string& engine, const std::string& version,
    const std::string& variant_name, bool async) {

  // check if engine variant is installed
  bool is_installed = false;
  if (is_installed) {
    // set default
    // TODO: namh implement this
    return {};
  }

  // TODO: namh add back the github_token

  auto normalized_version = string_utils::RemoveSubstring(version, "v");
  auto merged_variant_name =
      engine + "-" + normalized_version + "-" + variant_name + ".tar.gz";
  auto res = GetEngineVariants(engine, version);
  if (res.has_error()) {
    return cpp::fail("Failed to fetch engine releases: " + res.error());
  }

  if (res.value().empty()) {
    return cpp::fail("No release found for " + version);
  }

  std::optional<EngineVariant> selected_variant = std::nullopt;
  for (const auto& asset : res.value()) {
    if (asset.name == merged_variant_name) {
      selected_variant = asset;
      break;
    }
  }

  if (selected_variant == std::nullopt) {
    return cpp::fail("Not found variant: " + variant_name);
  }

  auto engine_folder_path =
      file_manager_utils::GetEnginesContainerPath() / engine;
  auto variant_folder_path = engine_folder_path / variant_name / version;
  auto variant_path = variant_folder_path / merged_variant_name;
  std::filesystem::create_directories(variant_folder_path);
  CLI_LOG("variant_folder_path: " + variant_folder_path.string());

  auto on_finished = [](const DownloadTask& finishedTask) {
    // try to unzip the downloaded file
    CLI_LOG("Engine zip path: " << finishedTask.items[0].localPath.string());

    auto extract_path = finishedTask.items[0].localPath.parent_path();

    archive_utils::ExtractArchive(finishedTask.items[0].localPath.string(),
                                  extract_path.string(), true);

    // remove the downloaded file
    try {
      std::filesystem::remove(finishedTask.items[0].localPath);
    } catch (const std::exception& e) {
      CTL_WRN("Could not delete file: " << e.what());
    }
    CTL_INF("Finished!");
  };

  auto downloadTask{
      DownloadTask{.id = engine,
                   .type = DownloadType::Engine,
                   .items = {DownloadItem{
                       .id = engine,
                       .downloadUrl = selected_variant->browser_download_url,
                       .localPath = variant_path,
                   }}}};

  auto add_task_result = download_service_->AddTask(downloadTask, on_finished);
  if (res.has_error()) {
    return cpp::fail(res.error());
  }
  return {};
}

cpp::result<bool, std::string> EngineService::DownloadEngine(
    const std::string& engine, const std::string& version, bool async) {

  // auto get_params = [&engine, &version]() -> std::vector<std::string> {
  //   if (version == "latest") {
  //     return {"repos", "janhq", engine, "releases", version};
  //   } else {
  //     return {"repos", "janhq", engine, "releases"};
  //   }
  // };
  //
  // auto url_obj = url_parser::Url{
  //     .protocol = "https",
  //     .host = "api.github.com",
  //     .pathParams = get_params(),
  // };

  // std::unordered_map<std::string, std::string> headers;

  // Check if GITHUB_TOKEN env exist
  // const char* github_token = std::getenv("GITHUB_TOKEN");
  // if (github_token) {
  //   std::string auth_header = "token " + std::string(github_token);
  //   headers.insert({"Authorization", auth_header});
  //   CTL_INF("Using authentication with GitHub token.");
  // } else {
  //   CTL_INF("No GitHub token found. Sending request without authentication.");
  // }

  auto res = GetEngineVariants(engine, version);
  if (res.has_error()) {
    return cpp::fail("Failed to fetch engine releases: " + res.error());
  }

  if (res.value().empty()) {
    return cpp::fail("No release found for " + version);
  }

  auto os_arch{hw_inf_.sys_inf->os + "-" + hw_inf_.sys_inf->arch};

  std::vector<std::string> variants;
  for (const auto& asset : res.value()) {
    variants.push_back(asset.name);
  }

  CTL_INF("engine: " << engine);
  CTL_INF("CUDA version: " << hw_inf_.cuda_driver_version);
  auto matched_variant = GetMatchedVariant(engine, variants);
  CTL_INF("Matched variant: " << matched_variant);
  if (matched_variant.empty()) {
    CTL_ERR("No variant found for " << os_arch);
    return cpp::fail("No variant found for " + os_arch);
  }

  for (const auto& asset : res.value()) {
    if (asset.name == matched_variant) {
      CTL_INF("Download url: " << asset.browser_download_url);

      std::filesystem::path engine_folder_path =
          file_manager_utils::GetContainerFolderPath(
              file_manager_utils::DownloadTypeToString(DownloadType::Engine)) /
          engine;

      if (!std::filesystem::exists(engine_folder_path)) {
        CTL_INF("Creating " << engine_folder_path.string());
        std::filesystem::create_directories(engine_folder_path);
      }

      CTL_INF("Engine folder path: " << engine_folder_path.string() << "\n");
      auto local_path = engine_folder_path / asset.name;
      auto downloadTask{
          DownloadTask{.id = engine,
                       .type = DownloadType::Engine,
                       .items = {DownloadItem{
                           .id = engine,
                           .downloadUrl = asset.browser_download_url,
                           .localPath = local_path,
                       }}}};

      auto on_finished = [](const DownloadTask& finishedTask) {
        // try to unzip the downloaded file
        CTL_INF(
            "Engine zip path: " << finishedTask.items[0].localPath.string());

        std::filesystem::path extract_path =
            finishedTask.items[0].localPath.parent_path().parent_path();

        archive_utils::ExtractArchive(finishedTask.items[0].localPath.string(),
                                      extract_path.string());

        // remove the downloaded file
        try {
          std::filesystem::remove(finishedTask.items[0].localPath);
        } catch (const std::exception& e) {
          CTL_WRN("Could not delete file: " << e.what());
        }
        CTL_INF("Finished!");
      };

      if (async) {
        auto res = download_service_->AddTask(downloadTask, on_finished);
        if (res.has_error()) {
          return cpp::fail(res.error());
        }
        return true;
      } else {
        return download_service_->AddDownloadTask(downloadTask, on_finished);
      }
    }
  }
  return true;
}

cpp::result<bool, std::string> EngineService::DownloadCuda(
    const std::string& engine, bool async) {
  if (hw_inf_.sys_inf->os == "mac" || engine == kOnnxRepo ||
      engine == kOnnxEngine) {
    // mac and onnx engine does not require cuda toolkit
    return true;
  }

  if (hw_inf_.cuda_driver_version.empty()) {
    CTL_WRN("No cuda driver, continue with CPU");
    return true;
  }
  // download cuda toolkit
  const std::string jan_host = "catalog.jan.ai";
  const std::string cuda_toolkit_file_name = "cuda.tar.gz";
  const std::string download_id = "cuda";

  auto suitable_toolkit_version =
      GetSuitableCudaVersion(engine, hw_inf_.cuda_driver_version);

  // compare cuda driver version with cuda toolkit version
  // cuda driver version should be greater than toolkit version to ensure compatibility
  if (semantic_version_utils::CompareSemanticVersion(
          hw_inf_.cuda_driver_version, suitable_toolkit_version) < 0) {
    CTL_ERR("Your Cuda driver version "
            << hw_inf_.cuda_driver_version
            << " is not compatible with cuda toolkit version "
            << suitable_toolkit_version);
    return cpp::fail("Cuda driver is not compatible with cuda toolkit");
  }

  auto url_obj = url_parser::Url{
      .protocol = "https",
      .host = jan_host,
      .pathParams = {"dist", "cuda-dependencies", suitable_toolkit_version,
                     hw_inf_.sys_inf->os, cuda_toolkit_file_name},
  };

  auto cuda_toolkit_url = url_parser::FromUrl(url_obj);

  LOG_DEBUG << "Cuda toolkit download url: " << cuda_toolkit_url;
  auto cuda_toolkit_local_path =
      file_manager_utils::GetContainerFolderPath(
          file_manager_utils::DownloadTypeToString(DownloadType::CudaToolkit)) /
      cuda_toolkit_file_name;
  LOG_DEBUG << "Download to: " << cuda_toolkit_local_path.string();
  auto downloadCudaToolkitTask{DownloadTask{
      .id = download_id,
      .type = DownloadType::CudaToolkit,
      .items = {DownloadItem{.id = download_id,
                             .downloadUrl = cuda_toolkit_url,
                             .localPath = cuda_toolkit_local_path}},
  }};

  auto on_finished = [engine](const DownloadTask& finishedTask) {
    auto engine_path = file_manager_utils::GetEnginesContainerPath() / engine;
    archive_utils::ExtractArchive(finishedTask.items[0].localPath.string(),
                                  engine_path.string());

    try {
      std::filesystem::remove(finishedTask.items[0].localPath);
    } catch (std::exception& e) {
      CTL_ERR("Error removing downloaded file: " << e.what());
    }
  };
  if (async) {
    auto res = download_service_->AddTask(downloadCudaToolkitTask, on_finished);
    if (res.has_error()) {
      return cpp::fail(res.error());
    }
    return true;
  } else {
    return download_service_->AddDownloadTask(downloadCudaToolkitTask,
                                              on_finished);
  }
}

std::string EngineService::GetMatchedVariant(
    const std::string& engine, const std::vector<std::string>& variants) {
  std::string matched_variant;
  if (engine == kTrtLlmRepo || engine == kTrtLlmEngine) {
    matched_variant = engine_matcher_utils::ValidateTensorrtLlm(
        variants, hw_inf_.sys_inf->os, hw_inf_.cuda_driver_version);
  } else if (engine == kOnnxRepo || engine == kOnnxEngine) {
    matched_variant = engine_matcher_utils::ValidateOnnx(
        variants, hw_inf_.sys_inf->os, hw_inf_.sys_inf->arch);
  } else if (engine == kLlamaRepo || engine == kLlamaEngine) {
    auto suitable_avx =
        engine_matcher_utils::GetSuitableAvxVariant(hw_inf_.cpu_inf);
    matched_variant = engine_matcher_utils::Validate(
        variants, hw_inf_.sys_inf->os, hw_inf_.sys_inf->arch, suitable_avx,
        hw_inf_.cuda_driver_version);
  }
  return matched_variant;
}

cpp::result<std::vector<EngineService::EngineRelease>, std::string>
EngineService::GetEngineReleases(const std::string& engine) const {
  auto ne = NormalizeEngine(engine);
  return github_release_utils::GetReleases("janhq", ne);
}

cpp::result<std::vector<EngineService::EngineVariant>, std::string>
EngineService::GetEngineVariants(const std::string& engine,
                                 const std::string& version) const {
  auto ne = NormalizeEngine(engine);
  auto engine_release =
      github_release_utils::GetReleaseByVersion("janhq", ne, version);

  if (engine_release.has_error()) {
    return cpp::fail("Failed to get engine release: " + engine_release.error());
  }

  std::vector<EngineVariant> compatible_variants;
  for (const auto& variant : engine_release.value().assets) {
    if (variant.content_type != "application/gzip") {
      continue;
    }
    if (variant.state != "uploaded") {
      continue;
    }
    compatible_variants.push_back(variant);
  }

  if (compatible_variants.empty()) {
    return cpp::fail("No compatible variants found for " + engine);
  }

  return compatible_variants;
}

cpp::result<DefaultEngineVariant, std::string>
EngineService::SetDefaultEngineVariant(const std::string& engine,
                                       const std::string& version,
                                       const std::string& variant) {
  auto ne = NormalizeEngine(engine);
  auto is_installed = IsEngineVariantReady(engine, version, variant);
  if (is_installed.has_error()) {
    return cpp::fail(is_installed.error());
  }

  if (!is_installed.value()) {
    return cpp::fail("Engine variant " + version + "-" + variant +
                     " is not installed yet!");
  }

  default_variants_[ne] = DefaultEngineVariant{
      .engine = engine,
      .version = version,
      .variant = variant,
  };
  return default_variants_[ne];
}

cpp::result<bool, std::string> EngineService::IsEngineVariantReady(
    const std::string& engine, const std::string& version,
    const std::string& variant) {
  auto ne = NormalizeEngine(engine);
  auto normalized_version = string_utils::RemoveSubstring(version, "v");
  auto installed_engines = GetInstalledEngineVariants(ne);

  for (const auto& installed_engine : installed_engines) {
    if (installed_engine.name == variant &&
        installed_engine.version == normalized_version) {
      return true;
    }
  }
  return false;
}

cpp::result<DefaultEngineVariant, std::string>
EngineService::GetDefaultEngineVariant(const std::string& engine) {
  auto ne = NormalizeEngine(engine);
  if (default_variants_.find(ne) == default_variants_.end()) {
    return cpp::fail("Engine variant for " + engine + " is not set yet!");
  }

  return default_variants_[ne];
}

std::vector<EngineVariantResponse> EngineService::GetInstalledEngineVariants(
    const std::string& engine) const {
  auto ne = NormalizeEngine(engine);
  auto engines_variants_dir =
      file_manager_utils::GetEnginesContainerPath() / ne;

  if (!std::filesystem::exists(engines_variants_dir)) {
    return {};
  }

  std::vector<EngineVariantResponse> variants;
  for (const auto& entry :
       std::filesystem::directory_iterator(engines_variants_dir)) {
    if (entry.is_directory()) {
      // epectation is each directory is a variant
      for (const auto& version_entry :
           std::filesystem::directory_iterator(entry.path())) {
        // try to find version.txt
        auto version_txt_path = version_entry.path() / "version.txt";
        if (!std::filesystem::exists(version_txt_path)) {
          continue;
        }

        try {
          auto node = YAML::LoadFile(version_txt_path.string());
          auto ev = EngineVariantResponse{
              .name = node["name"].as<std::string>(),
              .version = node["version"].as<std::string>(),
              .engine = engine,
          };
          variants.push_back(ev);
        } catch (const YAML::Exception& e) {
          CTL_ERR("Error reading version.txt: " << e.what());
          continue;
        }
      }
    }
  }

  return variants;
}

bool EngineService::IsEngineLoaded(const std::string& engine) const {
  auto ne = NormalizeEngine(engine);
  return engines_.find(ne) != engines_.end();
}

cpp::result<EngineV, std::string> EngineService::GetLoadedEngine(
    const std::string& engine_name) {
  auto ne = NormalizeEngine(engine_name);
  if (engines_.find(ne) == engines_.end()) {
    return cpp::fail("Engine " + engine_name + " is not loaded yet!");
  }

  return engines_[ne].engine;
}

cpp::result<void, std::string> EngineService::LoadEngine(
    const std::string& engine_name) {
  auto ne = NormalizeEngine(engine_name);

  if (IsEngineLoaded(ne)) {
    CTL_INF("Engine " << ne << " is already loaded");
    return {};
  }

  CTL_INF("Loading engine: " << ne);

  auto selected_engine_variant = GetDefaultEngineVariant(ne);

  if (selected_engine_variant.has_error()) {
    // TODO: namh need to fallback
    return cpp::fail(selected_engine_variant.error());
  }

  CTL_INF("Selected engine variant: "
          << json_helper::DumpJsonString(selected_engine_variant->ToJson()));

  auto user_defined_engine_path = getenv("ENGINE_PATH");
  const std::filesystem::path engine_dir_path = [&] {
    if (user_defined_engine_path != nullptr) {
      // for backward compatible
      return std::filesystem::path(user_defined_engine_path +
                                   GetEnginePath(ne));
    } else {
      return file_manager_utils::GetEnginesContainerPath() / ne /
             selected_engine_variant->variant /
             selected_engine_variant->version;
    }
  }();

  if (!std::filesystem::exists(engine_dir_path)) {
    CTL_ERR("Directory " + engine_dir_path.string() + " is not exist!");
    return cpp::fail("Directory " + engine_dir_path.string() +
                     " is not exist!");
  }

  CTL_INF("Engine path: " << engine_dir_path.string());

  try {
#if defined(_WIN32)
    // TODO(?) If we only allow to load an engine at a time, the logic is simpler.
    // We would like to support running multiple engines at the same time. Therefore,
    // the adding/removing dll directory logic is quite complicated:
    // 1. If llamacpp is loaded and new requested engine is tensorrt-llm:
    // Unload the llamacpp dll directory then load the tensorrt-llm
    // 2. If tensorrt-llm is loaded and new requested engine is llamacpp:
    // Do nothing, llamacpp can re-use tensorrt-llm dependencies (need to be tested careful)
    // 3. Add dll directory if met other conditions

    auto add_dll = [this](const std::string& e_type, const std::string& p) {
      auto ws = std::wstring(p.begin(), p.end());
      if (auto cookie = AddDllDirectory(ws.c_str()); cookie != 0) {
        LOG_INFO << "Added dll directory: " << p;
        engines_[e_type].cookie = cookie;
      } else {
        LOG_WARN << "Could not add dll directory: " << p;
      }
    };

    if (bool should_use_dll_search_path = !(getenv("ENGINE_PATH"));
        should_use_dll_search_path) {
      if (IsEngineLoaded(kLlamaRepo) && ne == kTrtLlmRepo &&
          should_use_dll_search_path) {
        // Remove llamacpp dll directory
        if (!RemoveDllDirectory(engines_[kLlamaRepo].cookie)) {
          LOG_WARN << "Could not remove dll directory: " << kLlamaRepo;
        } else {
          LOG_INFO << "Removed dll directory: " << kLlamaRepo;
        }

        add_dll(ne, engine_dir_path.string());
      } else if (IsEngineLoaded(kTrtLlmRepo) && ne == kLlamaRepo) {
        // Do nothing
      } else {
        add_dll(ne, engine_dir_path.string());
      }
    }
#endif
    engines_[ne].dl =
        std::make_unique<cortex_cpp::dylib>(engine_dir_path.string(), "engine");

  } catch (const cortex_cpp::dylib::load_error& e) {
    LOG_ERROR << "Could not load engine: " << e.what();
    engines_.erase(ne);
    return cpp::fail("Could not load engine " + ne + ": " + e.what());
  }

  auto func = engines_[ne].dl->get_function<EngineI*()>("get_engine");
  engines_[ne].engine = func();

  auto& en = std::get<EngineI*>(engines_[ne].engine);
  if (ne == kLlamaRepo) {  //fix for llamacpp engine first
    auto config = file_manager_utils::GetCortexConfig();
    // TODO: crash issue with trantor logging destructor.
    // if (en->IsSupported("SetFileLogger")) {
    //   en->SetFileLogger(config.maxLogLines,
    //                     (std::filesystem::path(config.logFolderPath) /
    //                      std::filesystem::path(config.logLlamaCppPath))
    //                         .string());
    // } else {
    //   LOG_WARN << "Method SetFileLogger is not supported yet";
    // }
  }
  LOG_INFO << "Loaded engine: " << ne;
  return {};
}

cpp::result<void, std::string> EngineService::UnloadEngine(
    const std::string& engine) {
  auto ne = NormalizeEngine(engine);
  if (!IsEngineLoaded(ne)) {
    return cpp::fail("Engine " + ne + " is not loaded yet!");
  }
  EngineI* e = std::get<EngineI*>(engines_[ne].engine);
  delete e;
#if defined(_WIN32)
  if (!RemoveDllDirectory(engines_[ne].cookie)) {
    LOG_WARN << "Could not remove dll directory: " << ne;
  } else {
    LOG_INFO << "Removed dll directory: " << ne;
  }
#endif
  engines_.erase(ne);
  LOG_INFO << "Unloaded engine " + ne;
  return {};
}

std::vector<EngineV> EngineService::GetLoadedEngines() {
  std::vector<EngineV> loaded_engines;
  for (const auto& [key, value] : engines_) {
    loaded_engines.push_back(value.engine);
  }
  return loaded_engines;
}

cpp::result<github_release_utils::GitHubRelease, std::string>
EngineService::GetLatestEngineVersion(const std::string& engine) const {
  auto ne = NormalizeEngine(engine);
  auto res = github_release_utils::GetReleaseByVersion("janhq", ne, "latest");
  if (res.has_error()) {
    return cpp::fail("Failed to fetch engine " + engine + " latest version!");
  }
  return res.value();
}

cpp::result<bool, std::string> EngineService::IsEngineReady(
    const std::string& engine) const {
  auto ne = NormalizeEngine(engine);
  auto installed_variants = GetInstalledEngineVariants(engine);

  auto os = hw_inf_.sys_inf->os;
  if (os == kMacOs && (ne == kOnnxRepo || ne == kTrtLlmRepo)) {
    return cpp::fail("Engine " + engine + " is not supported on macOS");
  }

  if (os == kLinuxOs && ne == kOnnxRepo) {
    return cpp::fail("Engine " + engine + " is not supported on Linux");
  }

  return installed_variants.size() > 0;
}

cpp::result<EngineUpdateResult, std::string> EngineService::UpdateEngine(
    const std::string& engine) {
  auto ne = NormalizeEngine(engine);
  auto default_variant = GetDefaultEngineVariant(ne);

  if (default_variant.has_error()) {
    // if we don't have a default variant, just stop
    CTL_INF("No default variant found for " << ne << ". Exit update engine");
    return cpp::fail(default_variant.error());
  }
  CTL_INF("Default variant: " << default_variant->variant
                              << ", version: " + default_variant->version);

  auto latest_version = GetLatestEngineVersion(ne);
  if (latest_version.has_error()) {
    // if can't get latest version, stop
    CTL_INF("Can't get latest version for "
            << ne << " error: " << latest_version.error());
    return cpp::fail("Failed to get latest version: " + latest_version.error());
  }
  CTL_INF("Latest version: " + latest_version.value().name);

  // check if local engines variants if latest version already exist
  auto installed_variants = GetInstalledEngineVariants(ne);

  bool is_installed = false;
  for (const auto& v : installed_variants) {
    CTL_INF("Installed version: " + v.version);
    if (default_variant->variant == v.name &&
        v.version == latest_version.value().name) {
      is_installed = true;
      break;
    }
  }

  if (is_installed) {
    CTL_INF("Engine " + ne + ", " + default_variant->variant +
            " is already up-to-date! Version " +
            latest_version.value().tag_name);
    return cpp::fail("Engine " + ne + ", " + default_variant->variant +
                     " is already up-to-date! Version " +
                     latest_version.value().tag_name);
  }

  CTL_INF("Engine variant "
          << default_variant->variant << " is not up-to-date! Current: "
          << default_variant->version << ", latest: " << latest_version->name);

  auto res = InstallEngineAsyncV2(engine, latest_version->tag_name,
                                  default_variant->variant);

  return EngineUpdateResult{.engine = engine,
                            .variant = default_variant->variant,
                            .from = default_variant->version,
                            .to = latest_version->name};
}
