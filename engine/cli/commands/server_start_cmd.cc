#include "server_start_cmd.h"
#include "commands/cortex_upd_cmd.h"
#include "services/engine_service.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/process/utils.h"

#if defined(_WIN32) || defined(_WIN64)
#include "utils/widechar_conv.h"
#endif

namespace commands {

namespace {
bool TryConnectToServer(const std::string& host, int port) {
  constexpr const auto kMaxRetry = 4u;
  auto count = 0u;
  // Check if server is started
  while (true) {
    if (IsServerAlive(host, port))
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

bool ServerStartCmd::Exec(const std::string& host, int port,
                          const std::optional<std::string>& log_level) {
  if (IsServerAlive(host, port)) {
    CLI_LOG("The server has already started");
    return true;
  }
  std::string log_level_;
  if (!log_level.has_value()) {
    log_level_ = "INFO";
  } else {
    log_level_ = log_level.value();
  }
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

#if defined(_WIN32) || defined(_WIN64)
  // Windows-specific code to create a new process
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));
  std::wstring params = L"--start-server";
  params += L" --config_file_path \"" +
            file_manager_utils::GetConfigurationPath().wstring() + L"\"";
  params += L" --data_folder_path \"" +
            file_manager_utils::GetCortexDataPath().wstring() + L"\"";
  params += L" --loglevel " + cortex::wc::Utf8ToWstring(log_level_);
  std::wstring exe_w = cortex::wc::Utf8ToWstring(exe);
  std::wstring current_path_w =
      file_manager_utils::GetExecutableFolderContainerPath().wstring();
  std::wstring wcmds = current_path_w + L"\\" + exe_w + L" " + params;
  CTL_INF("wcmds: " << wcmds);
  std::vector<wchar_t> mutable_cmds(wcmds.begin(), wcmds.end());
  mutable_cmds.push_back(L'\0');
  // Create child process
  if (!CreateProcess(
          NULL,  // No module name (use command line)
          mutable_cmds
              .data(),  // Command line (replace with your actual executable)
          NULL,         // Process handle not inheritable
          NULL,         // Thread handle not inheritable
          FALSE,        // Set handle inheritance
          CREATE_NO_WINDOW,  // No new console
          NULL,              // Use parent's environment block
          NULL,              // Use parent's starting directory
          &si,               // Pointer to STARTUPINFO structure
          &pi))              // Pointer to PROCESS_INFORMATION structure
  {
    std::cout << "Could not start server: " << GetLastError() << std::endl;
    return false;
  } else {
    if (!TryConnectToServer(host, port)) {
      return false;
    }
    std::cout << "Server started" << std::endl;
    std::cout << "API Documentation available at: http://" << host << ":"
              << port << std::endl;
  }

#else
  std::vector<std::string> commands;
  // Some engines requires to add lib search path before process being created
  EngineService(std::make_shared<cortex::DylibPathManager>())
      .RegisterEngineLibPath();

  std::string p = cortex_utils::GetCurrentPath() + "/" + exe;
  commands.push_back(p);
  commands.push_back("--config_file_path");
  commands.push_back(get_config_file_path());
  commands.push_back("--data_folder_path");
  commands.push_back(get_data_folder_path());
  commands.push_back("--loglevel");
  commands.push_back(log_level_);
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
    std::cout << "Server started" << std::endl;
    std::cout << "API Documentation available at: http://" << host << ":"
              << port << std::endl;
  }
#endif
  return true;
}

bool ServerStartCmd::Exec(
    const std::optional<std::string>& log_level,
    const std::unordered_map<std::string, std::string>& options,
    CortexConfig& data) {
  for (const auto& [key, value] : options) {
    if (!value.empty()) {
      UpdateConfig(data, key, value);
    }
  }

  auto config_path = file_manager_utils::GetConfigurationPath();
  auto result =
      config_yaml_utils::CortexConfigMgr::GetInstance().DumpYamlConfig(
          data, config_path.string());
  if (result.has_error()) {
    CTL_WRN("Error update " << config_path.string() << result.error());
  }
  return Exec(data.apiServerHost, std::stoi(data.apiServerPort), log_level);
}

void ServerStartCmd::UpdateConfig(CortexConfig& data, const std::string& key,
                                  const std::string& value) {
  static const std::unordered_map<
      std::string, std::function<void(CortexConfig&, const std::string&,
                                      const std::string&)>>
      updaters = {
          {"logspath",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.logFolderPath = v;
           }},
          {"logsllama",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.logLlamaCppPath = v;
           }},
          {"logsonnx",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.logOnnxPath = v;
           }},
          {"loglines",
           [this](CortexConfig& data, const std::string& k,
                  const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data.maxLogLines = static_cast<int>(f);
             });
           }},
          {"host",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.apiServerHost = v;
           }},
          {"port",
           [](CortexConfig& data, const std::string& k, const std::string& v) {
             data.apiServerPort = v;
             (void)k;
           }},
          {"hf-token",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.huggingFaceToken = v;
           }},
          {"gh-agent",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.gitHubUserAgent = v;
           }},
          {"gh-token",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.gitHubToken = v;
           }},
          {"cors",
           [this](CortexConfig& data, const std::string& k,
                  const std::string& v) {
             UpdateBooleanField(k, v, [&data](bool b) { data.enableCors = b; });
           }},
          {"origins",
           [this](CortexConfig& data, const std::string& k,
                  const std::string& v) {
             UpdateVectorField(k, v,
                               [&data](const std::vector<std::string>& orgs) {
                                 data.allowedOrigins = orgs;
                               });
           }},
          {"proxy-url",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.proxyUrl = v;
           }},
          {"verify-proxy",
           [this](CortexConfig& data, const std::string& k,
                  const std::string& v) {
             UpdateBooleanField(k, v,
                                [&data](bool b) { data.verifyProxySsl = b; });
           }},
          {"verify-proxy-host",
           [this](CortexConfig& data, const std::string& k,
                  const std::string& v) {
             UpdateBooleanField(
                 k, v, [&data](bool b) { data.verifyProxyHostSsl = b; });
           }},
          {"proxy-username",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.proxyUsername = v;
           }},
          {"proxy-password",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.proxyPassword = v;
           }},
          {"no-proxy",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.noProxy = v;
           }},
          {"verify-ssl-peer",
           [this](CortexConfig& data, const std::string& k,
                  const std::string& v) {
             UpdateBooleanField(k, v,
                                [&data](bool b) { data.verifyPeerSsl = b; });
           }},
          {"verify-ssl-host",
           [this](CortexConfig& data, const std::string& k,
                  const std::string& v) {
             UpdateBooleanField(k, v,
                                [&data](bool b) { data.verifyHostSsl = b; });
           }},
          {"ssl-cert-path",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.sslCertPath = v;
           }},
          {"ssl-key-path",
           [](CortexConfig& data, const std::string&, const std::string& v) {
             data.sslKeyPath = v;
           }},
      };

  if (auto it = updaters.find(key); it != updaters.end()) {
    it->second(data, key, value);
    CTL_INF("Updated " << key << " to: " << value);
  } else {
    CTL_WRN("Warning: Unknown configuration key '" << key << "' ignored.");
  }
}

void ServerStartCmd::UpdateVectorField(
    const std::string& key, const std::string& value,
    std::function<void(const std::vector<std::string>&)> setter) {
  std::vector<std::string> tokens;
  std::istringstream iss(value);
  std::string token;
  while (std::getline(iss, token, ',')) {
    tokens.push_back(token);
  }
  setter(tokens);
  (void)key;
}

void ServerStartCmd::UpdateNumericField(const std::string& key,
                                        const std::string& value,
                                        std::function<void(float)> setter) {
  try {
    float numeric_val = std::stof(value);
    setter(numeric_val);
  } catch (const std::exception& e) {
    CLI_LOG("Failed to parse numeric value for " << key << ": " << e.what());
  }
}

void ServerStartCmd::UpdateBooleanField(const std::string& key,
                                        const std::string& value,
                                        std::function<void(bool)> setter) {
  bool bool_value = (value == "true" || value == "1");
  setter(bool_value);
  (void)key;
}

};  // namespace commands
