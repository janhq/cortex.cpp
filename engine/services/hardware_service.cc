// clang-format off
#include "cli/commands/server_start_cmd.h"
// clang-format on
#include "hardware_service.h"
#if defined(_WIN32) || defined(_WIN64)
#include <minwindef.h>
#include <processenv.h>
#endif
#include "cli/commands/cortex_upd_cmd.h"
#include "database/hardwares.h"
#include "services/engine_service.h"
#include "utils/cortex_utils.h"

namespace services {

namespace {
bool TryConnectToServer(const std::string& host, int port) {
  constexpr const auto kMaxRetry = 3u;
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
  cortex::db::Hardwares hw_db;
  auto gpus = hardware::GetGPUInfo();
  auto res = hw_db.LoadHardwareList();
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

  return HardwareInfo{.cpu = hardware::GetCPUInfo(),
                      .os = hardware::GetOSInfo(),
                      .ram = hardware::GetMemoryInfo(),
                      .storage = hardware::GetStorageInfo(),
                      .gpus = gpus,
                      .power = hardware::GetPowerInfo()};
}

bool HardwareService::Restart(const std::string& host, int port) {
  if (!ahc_)
    return true;
  auto exe = commands::GetCortexServerBinary();
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
  for (auto i : (*ahc_).gpus) {
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
#endif

#if defined(_WIN32) || defined(_WIN64)
  // Windows-specific code to create a new process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  std::string params = "--ignore_cout";
  params += " --config_file_path " + get_config_file_path();
  params += " --data_folder_path " + get_data_folder_path();
  std::string cmds = cortex_utils::GetCurrentPath() + "/" + exe + " " + params;
  // Create child process
  if (!CreateProcess(
          NULL,  // No module name (use command line)
          const_cast<char*>(
              cmds.c_str()),  // Command line (replace with your actual executable)
          NULL,               // Process handle not inheritable
          NULL,               // Thread handle not inheritable
          TRUE,               // Handle inheritance
          0,                  // No creation flags
          NULL,               // Use parent's environment block
          NULL,               // Use parent's starting directory
          &si,                // Pointer to STARTUPINFO structure
          &pi))               // Pointer to PROCESS_INFORMATION structure
  {
    std::cout << "Could not start server: " << GetLastError() << std::endl;
    return false;
  } else {
    if (!TryConnectToServer(host, port)) {
      return false;
    }
  }

#else
  // Unix-like system-specific code to fork a child process
  pid_t pid = fork();

  if (pid < 0) {
    // Fork failed
    std::cerr << "Could not start server: " << std::endl;
    return false;
  } else if (pid == 0) {
    // No need to configure LD_LIBRARY_PATH for macOS
#if !defined(__APPLE__) || !defined(__MACH__)
    const char* name = "LD_LIBRARY_PATH";
    auto data = getenv(name);
    std::string v;
    if (auto g = getenv(name); g) {
      v += g;
    }
    CTL_INF("LD_LIBRARY_PATH: " << v);
    auto llamacpp_path = file_manager_utils::GetCudaToolkitPath(kLlamaRepo);
    auto trt_path = file_manager_utils::GetCudaToolkitPath(kTrtLlmRepo);

    auto new_v = trt_path.string() + ":" + llamacpp_path.string() + ":" + v;
    setenv(name, new_v.c_str(), true);
    CTL_INF("LD_LIBRARY_PATH: " << getenv(name));
#endif
    std::string p = cortex_utils::GetCurrentPath() + "/" + exe;
    execl(p.c_str(), exe.c_str(), "--ignore_cout", "--config_file_path",
          get_config_file_path().c_str(), "--data_folder_path",
          get_data_folder_path().c_str(), "--loglevel", "INFO", (char*)0);
  } else {
    // Parent process
    if (!TryConnectToServer(host, port)) {
      return false;
    }
  }
#endif
  return true;
}

bool HardwareService::SetActivateHardwareConfig(
    const cortex::hw::ActivateHardwareConfig& ahc) {
  // Note: need to map software_id and hardware_id
  // Update to db
  cortex::db::Hardwares hw_db;
  auto activate = [&ahc](int software_id) {
    return std::count(ahc.gpus.begin(), ahc.gpus.end(), software_id) > 0;
  };
  auto res = hw_db.LoadHardwareList();
  if (res.has_value()) {
    bool need_update = false;
    std::vector<int> activated_ids;
    // Check if need to update
    for (auto const& e : res.value()) {
      if (e.activated) {
        activated_ids.push_back(e.software_id);
      }
    }
    std::sort(activated_ids.begin(), activated_ids.end());
    if (ahc.gpus.size() != activated_ids.size()) {
      need_update = true;
    } else {
      for (size_t i = 0; i < ahc.gpus.size(); i++) {
        if (ahc.gpus[i] != activated_ids[i])
          need_update = true;
      }
    }

    if (!need_update) {
      CTL_INF("No hardware activation changes -> No need to update");
      return false;
    }

    // Need to update, proceed
    for (auto& e : res.value()) {
      e.activated = activate(e.software_id);
      auto res = hw_db.UpdateHardwareEntry(e.uuid, e);
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
  auto gpus = hardware::GetGPUInfo();
  cortex::db::Hardwares hw_db;
  auto b = hw_db.LoadHardwareList();
  std::vector<int> activated_gpu_bf;
  std::string debug_b;
  for (auto const& he : b.value()) {
    if (he.type == "gpu" && he.activated) {
      debug_b += std::to_string(he.software_id) + " ";
      activated_gpu_bf.push_back(he.software_id);
    }
  }
  CTL_INF("Activated GPUs before: " << debug_b);
  for (auto const& gpu : gpus) {
    // ignore error
    // Note: only support NVIDIA for now, so hardware_id = software_id
    auto res = hw_db.AddHardwareEntry(HwEntry{.uuid = gpu.uuid,
                                              .type = "gpu",
                                              .hardware_id = std::stoi(gpu.id),
                                              .software_id = std::stoi(gpu.id),
                                              .activated = true});
    if (res.has_error()) {
      CTL_WRN(res.error());
    }
  }

  auto a = hw_db.LoadHardwareList();
  std::vector<HwEntry> a_gpu;
  std::vector<int> activated_gpu_af;
  std::string debug_a;
  for (auto const& he : a.value()) {
    if (he.type == "gpu" && he.activated) {
      debug_a += std::to_string(he.software_id) + " ";
      activated_gpu_af.push_back(he.software_id);
    }
  }
  CTL_INF("Activated GPUs after: " << debug_a);
  // if hardware list changes, need to restart
  std::sort(activated_gpu_bf.begin(), activated_gpu_bf.end());
  std::sort(activated_gpu_af.begin(), activated_gpu_af.end());
  bool need_restart = false;
  if (activated_gpu_bf.size() != activated_gpu_af.size()) {
    need_restart = true;
  } else {
    for (size_t i = 0; i < activated_gpu_bf.size(); i++) {
      if (activated_gpu_bf[i] != activated_gpu_af[i]) {
        need_restart = true;
        break;
      }
    }
  }

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
  const char* value = std::getenv("CUDA_VISIBLE_DEVICES");
  if (value) {
    LOG_INFO << "CUDA_VISIBLE_DEVICES: " << value;
  } else {
    need_restart = true;
  }
#endif

  if (need_restart) {
    CTL_INF("Need restart");
    ahc_ = {.gpus = activated_gpu_af};
  }
}

bool HardwareService::IsValidConfig(
    const cortex::hw::ActivateHardwareConfig& ahc) {
  cortex::db::Hardwares hw_db;
  auto is_valid = [&ahc](int software_id) {
    return std::count(ahc.gpus.begin(), ahc.gpus.end(), software_id) > 0;
  };
  auto res = hw_db.LoadHardwareList();
  if (res.has_value()) {
    for (auto const& e : res.value()) {
      if (!is_valid(e.software_id)) {
        return false;
      }
    }
  }
  return true;
}
}  // namespace services