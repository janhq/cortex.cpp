// clang-format off
#include "cli/commands/server_start_cmd.h"
// clang-format on
#include "hardware_service.h"
#if defined(_WIN32) || defined(_WIN64)
#include <minwindef.h>
#include <processenv.h>
#include "utils/widechar_conv.h"
#endif
#include "cli/commands/cortex_upd_cmd.h"
#include "database/hardware.h"
#include "services/engine_service.h"
#include "utils/cortex_utils.h"
#include "utils/dylib_path_manager.h"
#include "utils/process/utils.h"
#if defined(__linux__)
#include "services/download_service.h"
#endif

namespace {
bool TryConnectToServer(const std::string& host, int port) {
  constexpr const auto kMaxRetry = 4u;
  auto count = 0u;
  // Check if server is started
  while (true) {
    if (commands::IsServerAlive(host, port))
      break;
    // Wait for server up
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (count++ == kMaxRetry) {
      std::cerr << "Could not start server" << std::endl;
      return false;
    }
  }
  return true;
}
}  // namespace

HardwareInfo HardwareService::GetHardwareInfo() {
  // append active state
  std::lock_guard<std::mutex> l(mtx_);
  auto gpus = cortex::hw::GetGPUInfo();
  auto res = db_service_->LoadHardwareList();
  if (res.has_value()) {
    // Only a few elements, brute-force is enough
    for (auto& entry : res.value()) {
      for (auto& gpu : gpus) {
        if (gpu.uuid == entry.uuid) {
          gpu.is_activated = entry.activated;
        }
      }
    };
  }

  return HardwareInfo{.cpu = cpu_info_.GetCPUInfo(),
                      .os = cortex::hw::GetOSInfo(),
                      .ram = cortex::hw::GetMemoryInfo(),
                      .storage = cortex::hw::GetStorageInfo(),
                      .gpus = gpus,
                      .power = cortex::hw::GetPowerInfo()};
}

bool HardwareService::Restart(const std::string& host, int port) {
  namespace luh = logging_utils_helper;
  if (!ahc_)
    return true;
  auto exe = file_manager_utils::Subtract(
      file_manager_utils::GetExecutablePath(), cortex_utils::GetCurrentPath());
  auto get_config_file_path = []() -> std::string {
    if (file_manager_utils::cortex_config_file_path.empty()) {
      return file_manager_utils::GetConfigurationPath().string();
    }
    return file_manager_utils::cortex_config_file_path;
  };

  auto get_data_folder_path = []() -> std::string {
    if (file_manager_utils::cortex_data_folder_path.empty()) {
      return file_manager_utils::GetCortexDataPath().string();
    }
    return file_manager_utils::cortex_data_folder_path;
  };

  auto set_env = [](const std::string& name, const std::string& value,
                    bool is_override = true) -> bool {
#if defined(_WIN32) || defined(_WIN64)
    return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), is_override) == 0;
#endif
  };

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
  std::string cuda_visible_devices = "";
  auto cuda_config = GetCudaConfig();
  for (auto i : cuda_config) {
    if (!cuda_visible_devices.empty())
      cuda_visible_devices += ",";
    cuda_visible_devices += std::to_string(i);
  }
  if (cuda_visible_devices.empty())
    cuda_visible_devices += " ";

  // Set the CUDA_VISIBLE_DEVICES environment variable
  if (!set_env("CUDA_VISIBLE_DEVICES", cuda_visible_devices)) {
    LOG_WARN << "Error setting CUDA_VISIBLE_DEVICES";
    return false;
  }

  const char* value = std::getenv("CUDA_VISIBLE_DEVICES");
  if (value) {
    LOG_INFO << "CUDA_VISIBLE_DEVICES is set to: " << value;
  } else {
    LOG_WARN << "CUDA_VISIBLE_DEVICES is not set.";
  }

  std::string vk_visible_devices = "";
  for (auto i : (*ahc_).gpus) {
    if (!vk_visible_devices.empty())
      vk_visible_devices += ",";
    vk_visible_devices += std::to_string(i);
  }

  if (vk_visible_devices.empty())
    vk_visible_devices += " ";

  if (!set_env("GGML_VK_VISIBLE_DEVICES", vk_visible_devices)) {
    LOG_WARN << "Error setting GGML_VK_VISIBLE_DEVICES";
    return false;
  }

  const char* vk_value = std::getenv("GGML_VK_VISIBLE_DEVICES");
  if (vk_value) {
    LOG_INFO << "GGML_VK_VISIBLE_DEVICES is set to: " << vk_value;
  } else {
    LOG_WARN << "GGML_VK_VISIBLE_DEVICES is not set.";
  }
#endif

#if defined(_WIN32) || defined(_WIN64)
  // Windows-specific code to create a new process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  // TODO (sang) write a common function for this and server_start_cmd
  std::wstring params = L"--ignore_cout";
  params += L" --config_file_path \"" +
            file_manager_utils::GetConfigurationPath().wstring() + L"\"";
  params += L" --data_folder_path \"" +
            file_manager_utils::GetCortexDataPath().wstring() + L"\"";
  params += L" --loglevel " +
            cortex::wc::Utf8ToWstring(luh::LogLevelStr(luh::global_log_level));
  std::wstring exe_w = exe.wstring();
  std::wstring current_path_w =
      file_manager_utils::GetExecutableFolderContainerPath().wstring();
  std::wstring wcmds = current_path_w + L"\\" + exe_w + L" " + params;
  CTL_DBG("wcmds: " << wcmds);
  std::vector<wchar_t> mutable_cmds(wcmds.begin(), wcmds.end());
  mutable_cmds.push_back(L'\0');
  // Create child process
  if (!CreateProcess(
          NULL,  // No module name (use command line)
          mutable_cmds
              .data(),  // Command line (replace with your actual executable)
          NULL,         // Process handle not inheritable
          NULL,         // Thread handle not inheritable
          TRUE,         // Handle inheritance
          0,            // No creation flags
          NULL,         // Use parent's environment block
          NULL,         // Use parent's starting directory
          &si,          // Pointer to STARTUPINFO structure
          &pi))         // Pointer to PROCESS_INFORMATION structure
  {
    std::cout << "Could not start server: " << GetLastError() << std::endl;
    return false;
  } else {
    if (!TryConnectToServer(host, port)) {
      return false;
    }
  }

#else
  std::vector<std::string> commands;
  // Some engines requires to add lib search path before process being created
  auto download_srv = std::make_shared<DownloadService>();
  auto dylib_path_mng = std::make_shared<cortex::DylibPathManager>();
  auto db_srv = std::make_shared<DatabaseService>();
  EngineService(download_srv, dylib_path_mng, db_srv).RegisterEngineLibPath();
  std::string p = cortex_utils::GetCurrentPath() / exe;
  commands.push_back(p);
  commands.push_back("--ignore_cout");
  commands.push_back("--config_file_path");
  commands.push_back(get_config_file_path());
  commands.push_back("--data_folder_path");
  commands.push_back(get_data_folder_path());
  commands.push_back("--loglevel");
  commands.push_back(luh::LogLevelStr(luh::global_log_level));
  auto result = cortex::process::SpawnProcess(commands);
  if (result.has_error()) {
    // Fork failed
    std::cerr << "Could not start server: " << result.error() << std::endl;
    return false;
  } else {
    // Parent process
    if (!TryConnectToServer(host, port)) {
      return false;
    }
  }

#endif
  return true;
}

// GPU identifiers are given as integer indices or as UUID strings. GPU UUID strings
// should follow the same format as given by nvidia-smi, such as GPU-8932f937-d72c-4106-c12f-20bd9faed9f6.
// However, for convenience, abbreviated forms are allowed; simply specify enough digits
// from the beginning of the GPU UUID to uniquely identify that GPU in the target system.
// For example, CUDA_VISIBLE_DEVICES=GPU-8932f937 may be a valid way to refer to the above GPU UUID,
// assuming no other GPU in the system shares this prefix. Only the devices whose index
// is present in the sequence are visible to CUDA applications and they are enumerated
// in the order of the sequence. If one of the indices is invalid, only the devices whose
// index precedes the invalid index are visible to CUDA applications. For example, setting
// CUDA_VISIBLE_DEVICES to 2,1 causes device 0 to be invisible and device 2 to be enumerated
// before device 1. Setting CUDA_VISIBLE_DEVICES to 0,2,-1,1 causes devices 0 and 2 to be
// visible and device 1 to be invisible. MIG format starts with MIG keyword and GPU UUID
// should follow the same format as given by nvidia-smi.
// For example, MIG-GPU-8932f937-d72c-4106-c12f-20bd9faed9f6/1/2.
// Only single MIG instance enumeration is supported.
bool HardwareService::SetActivateHardwareConfig(
    const cortex::hw::ActivateHardwareConfig& ahc) {
  // Note: need to map software_id and hardware_id
  // Update to db
  // copy all gpu information to new vector
  auto ahc_gpus = ahc.gpus;
  auto activate = [&ahc](int software_id) {
    return std::count(ahc.gpus.begin(), ahc.gpus.end(), software_id) > 0;
  };
  auto priority = [&ahc](int software_id) -> int {
    for (size_t i = 0; i < ahc.gpus.size(); i++) {
      if (ahc.gpus[i] == software_id)
        return i;
      break;
    }
    return INT_MAX;
  };

  auto res = db_service_->LoadHardwareList();
  if (res.has_value()) {
    bool need_update = false;
    std::vector<std::pair<int, int>> activated_ids;
    // Check if need to update
    for (auto const& e : res.value()) {
      if (e.activated) {
        activated_ids.push_back(std::pair(e.software_id, e.priority));
      }
    }
    std::sort(activated_ids.begin(), activated_ids.end(),
              [](auto& p1, auto& p2) { return p1.second < p2.second; });
    if (ahc_gpus.size() != activated_ids.size()) {
      need_update = true;
    } else {
      for (size_t i = 0; i < ahc_gpus.size(); i++) {
        // if activated id or priority changes
        if (ahc_gpus[i] != activated_ids[i].first ||
            i != activated_ids[i].second)
          need_update = true;
        break;
      }
    }

    if (!need_update) {
      CTL_INF("No hardware activation changes -> No need to update");
      return false;
    }

    // Need to update, proceed
    for (auto& e : res.value()) {
      e.activated = activate(e.software_id);
      e.priority = priority(e.software_id);
      auto res = db_service_->UpdateHardwareEntry(e.uuid, e);
      if (res.has_error()) {
        CTL_WRN(res.error());
      }
    }
  }
  ahc_ = ahc;
  return true;
}

void HardwareService::UpdateHardwareInfos() {
  using HwEntry = cortex::db::HardwareEntry;
  CheckDependencies();
  auto gpus = cortex::hw::GetGPUInfo();
  auto b = db_service_->LoadHardwareList();
  // delete if not exists
  auto exists = [&gpus](const std::string& uuid) {
    for (auto const& g : gpus) {
      if (g.uuid == uuid)
        return true;
    }
    return false;
  };
  for (auto const& he : b.value()) {
    if (!exists(he.uuid)) {
      db_service_->DeleteHardwareEntry(he.uuid);
    }
  }

  // Get updated list
  b = db_service_->LoadHardwareList();
  std::vector<std::pair<int, int>> activated_gpu_bf;
  std::string debug_b;
  for (auto const& he : b.value()) {
    if (he.type == "gpu" && he.activated) {
      debug_b += std::to_string(he.software_id) + " ";
      activated_gpu_bf.push_back(std::pair(he.software_id, he.priority));
    }
  }
  CTL_INF("Activated GPUs before: " << debug_b);
  auto has_nvidia = [&gpus] {
    for (auto const& g : gpus) {
      if (g.vendor == cortex::hw::kNvidiaStr) {
        return true;
      }
    }
    return false;
  }();

  for (auto const& gpu : gpus) {
    if (db_service_->HasHardwareEntry(gpu.uuid)) {
      auto res = db_service_->UpdateHardwareEntry(gpu.uuid, std::stoi(gpu.id),
                                                  std::stoi(gpu.id));
      if (res.has_error()) {
        CTL_WRN(res.error());
      }
    } else {
      // iGPU should be deactivated by default
      // Only activate Nvidia GPUs if both AMD and Nvidia GPUs exists
      auto activated = [&gpu, &gpus, has_nvidia] {
        if (gpu.gpu_type != cortex::hw::GpuType::kGpuTypeDiscrete)
          return false;
        if (has_nvidia && gpu.vendor != cortex::hw::kNvidiaStr)
          return false;
        return true;
      };

      auto res = db_service_->AddHardwareEntry(
          HwEntry{.uuid = gpu.uuid,
                  .type = "gpu",
                  .hardware_id = std::stoi(gpu.id),
                  .software_id = std::stoi(gpu.id),
                  .activated = activated(),
                  .priority = INT_MAX});
      if (res.has_error()) {
        CTL_WRN(res.error());
      }
    }
  }

  auto a = db_service_->LoadHardwareList();
  std::vector<HwEntry> a_gpu;
  std::vector<std::pair<int, int>> activated_gpu_af;
  std::string debug_a;
  for (auto const& he : a.value()) {
    if (he.type == "gpu" && he.activated) {
      debug_a += std::to_string(he.software_id) + " ";
      activated_gpu_af.push_back(std::pair(he.software_id, he.priority));
    }
  }
  CTL_INF("Activated GPUs after: " << debug_a);
  // if hardware list changes, need to restart
  std::sort(activated_gpu_bf.begin(), activated_gpu_bf.end(),
            [](auto& p1, auto& p2) { return p1.second < p2.second; });
  std::sort(activated_gpu_af.begin(), activated_gpu_af.end(),
            [](auto& p1, auto& p2) { return p1.second < p2.second; });
  bool need_restart = false;
  if (activated_gpu_bf.size() != activated_gpu_af.size()) {
    need_restart = true;
  } else {
    for (size_t i = 0; i < activated_gpu_bf.size(); i++) {
      if (activated_gpu_bf[i].first != activated_gpu_af[i].first) {
        need_restart = true;
        break;
      }
    }
  }

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
  bool has_deactivated_gpu = a.value().size() != activated_gpu_af.size();
  if (!gpus.empty() && has_deactivated_gpu) {
    const char* value = std::getenv("CUDA_VISIBLE_DEVICES");
    if (value) {
      LOG_INFO << "CUDA_VISIBLE_DEVICES: " << value;
    } else {
      need_restart = true;
    }

    const char* vk_value = std::getenv("GGML_VK_VISIBLE_DEVICES");
    if (vk_value) {
      LOG_INFO << "GGML_VK_VISIBLE_DEVICES: " << vk_value;
    } else {
      need_restart = true;
    }
  }
#endif

  if (need_restart) {
    CTL_INF("Need restart");
    std::vector<int> gpus;
    for (auto const& p : activated_gpu_af) {
      gpus.push_back(p.first);
    }
    ahc_ = {.gpus = gpus};
  }
}

bool HardwareService::IsValidConfig(
    const cortex::hw::ActivateHardwareConfig& ahc) {
  if (ahc.gpus.empty())
    return true;
  auto is_valid = [&ahc](int software_id) {
    return std::count(ahc.gpus.begin(), ahc.gpus.end(), software_id) > 0;
  };
  auto res = db_service_->LoadHardwareList();
  if (res.has_value()) {
    for (auto const& e : res.value()) {
      if (is_valid(e.software_id)) {
        return true;
      }
    }
  }
  return false;
}

void HardwareService::CheckDependencies() {
  // search for libvulkan.so, if does not exist, pull it
#if defined(__linux__)
  namespace fmu = file_manager_utils;
  auto get_vulkan_path = [](const std::string& lib_vulkan)
      -> cpp::result<std::filesystem::path, std::string> {
    if (std::filesystem::exists(fmu::GetExecutableFolderContainerPath() /
                                lib_vulkan)) {
      return fmu::GetExecutableFolderContainerPath() / lib_vulkan;
      // fallback to deps path
    } else if (std::filesystem::exists(fmu::GetCortexDataPath() / "deps" /
                                       lib_vulkan)) {
      return fmu::GetCortexDataPath() / "deps" / lib_vulkan;
    } else {
      CTL_WRN("Could not found " << lib_vulkan);
      return cpp::fail("Could not found " + lib_vulkan);
    }
  };

  if (get_vulkan_path("libvulkan.so").has_error()) {
    if (!std::filesystem::exists(fmu::GetCortexDataPath() / "deps")) {
      std::filesystem::create_directories(fmu::GetCortexDataPath() / "deps");
    }
    auto download_task{DownloadTask{
        .id = "vulkan",
        .type = DownloadType::Miscellaneous,
        .items = {DownloadItem{
            .id = "vulkan",
            .downloadUrl = "https://catalog.jan.ai/libvulkan.so",
            .localPath = fmu::GetCortexDataPath() / "deps" / "libvulkan.so",
        }},
    }};
    auto result = DownloadService().AddDownloadTask(
        download_task,
        [](const DownloadTask& finishedTask) {
          // try to unzip the downloaded file
          CTL_INF("Downloaded libvulkan path: "
                  << finishedTask.items[0].localPath.string());

          CTL_INF("Finished!");
        },
        /*show_progress*/ false);
    if (result.has_error()) {
      CTL_WRN("Failed to download: " << result.error());
    }
  }
#endif
}

std::vector<int> HardwareService::GetCudaConfig() {
  std::vector<int> res;
  if (!ahc_)
    return res;
  auto nvidia_gpus = system_info_utils::GetGpuInfoList();
  auto all_gpus = cortex::hw::GetGPUInfo();
  // Map id with uuid
  std::vector<std::string> uuids;
  for (auto i : (*ahc_).gpus) {
    for (auto const& gpu : all_gpus) {
      if (i == std::stoi(gpu.id)) {
        uuids.push_back(gpu.uuid);
      }
    }
  }

  // Map uuid back to nvidia id
  for (auto const& uuid : uuids) {
    for (auto const& ngpu : nvidia_gpus) {
      if (ngpu.uuid.find(uuid) != std::string::npos) {
        res.push_back(std::stoi(ngpu.id));
      }
    }
  }
  return res;
}
