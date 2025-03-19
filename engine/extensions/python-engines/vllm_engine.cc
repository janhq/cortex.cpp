#include "vllm_engine.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"

namespace {
cpp::result<std::string, std::string> GetLatestVllmVersion() {
  auto result = curl_utils::SimpleGetJson("https://pypi.org/pypi/vllm/json");
  if (result.has_error())
    return result.error();

  auto version_value = result.value()["info"]["version"];
  if (version_value.isNull())
    return cpp::fail("Can't find version in the response");

  return version_value.asString();
}
}  // namespace

VllmEngine::~VllmEngine() {
  // NOTE: what happens if we can't kill subprocess?
  std::unique_lock write_lock(mutex);
  for (auto& [model_name, py_proc] : model_process_map) {
    if (py_proc.IsAlive())
      py_proc.Kill();
  }
}

cpp::result<void, std::string> VllmEngine::Download(
    std::shared_ptr<DownloadService>& download_service,
    const std::string& version, const std::optional<std::string> variant_name) {
  if (variant_name.has_value()) {
    return cpp::fail("variant_name must be empty");
  }

  if (!python_utils::IsUvInstalled()) {
    auto result = python_utils::InstallUv(download_service);
    if (result.has_error())
      return result;
  }

  std::string concrete_version = version;
  if (version == "latest") {
    auto result = GetLatestVllmVersion();
    if (result.has_error())
      return cpp::fail(result.error());

    concrete_version = result.value();
  }
  CTL_INF("Download vLLM " << concrete_version);

  const auto vllm_path =
      python_utils::GetEnvsPath() / "vllm" / concrete_version;
  std::filesystem::create_directories(vllm_path);
  const auto vllm_path_str = vllm_path.string();

  // initialize venv
  if (!std::filesystem::exists(vllm_path / ".venv")) {
    std::vector<std::string> cmd =
        python_utils::BuildUvCommand("venv", vllm_path_str);
    auto result = cortex::process::SpawnProcess(cmd);
    if (result.has_error())
      return cpp::fail(result.error());

    // TODO: check return code
    // NOTE: these are not async
    cortex::process::WaitProcess(result.value());
  }

  // install vLLM
  {
    std::vector<std::string> cmd =
        python_utils::BuildUvCommand("pip", vllm_path_str);
    cmd.push_back("install");
    cmd.push_back("vllm==" + concrete_version);
    auto result = cortex::process::SpawnProcess(cmd);
    if (result.has_error())
      return cpp::fail(result.error());

    // TODO: check return code
    // NOTE: these are not async
    cortex::process::WaitProcess(result.value());
  }

  return {};
}

void VllmEngine::Load(EngineLoadOption opts) {};
void VllmEngine::Unload(EngineUnloadOption opts) {};

// cortex.llamacpp interface
void VllmEngine::HandleChatCompletion(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {};

void VllmEngine::HandleEmbedding(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {};

void VllmEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {};

void VllmEngine::UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {};

void VllmEngine::GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {};

// For backward compatible checking
bool VllmEngine::IsSupported(const std::string& f) {
  return true;
};

// Get list of running models
void VllmEngine::GetModels(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {};

bool VllmEngine::SetFileLogger(int max_log_lines, const std::string& log_path) {
  return true;
};
void VllmEngine::SetLogLevel(trantor::Logger::LogLevel logLevel) {};

// Stop inflight chat completion in stream mode
void VllmEngine::StopInferencing(const std::string& model_id) {};
