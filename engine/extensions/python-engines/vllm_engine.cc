#include "vllm_engine.h"
#include "services/engine_service.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"

static std::pair<Json::Value, Json::Value> CreateResponse(
    const std::string& msg, int code) {

  Json::Value status, res;
  const bool has_error = code != 200;

  status["is_done"] = true;
  status["has_error"] = has_error;
  status["is_stream"] = false;
  status["status_code"] = code;

  if (has_error) {
    CTL_ERR(msg);
    res["error"] = msg;
  } else {
    res["status"] = msg;
  }

  return {status, res};
}

VllmEngine::~VllmEngine() {
  // NOTE: what happens if we can't kill subprocess?
  std::unique_lock write_lock(mutex);
  for (auto& [model_name, py_proc] : model_process_map) {
    if (py_proc.IsAlive())
      py_proc.Kill();
  }
}

std::vector<EngineVariantResponse> VllmEngine::GetVariants() {
  const auto vllm_path = python_utils::GetEnvsPath() / "vllm";

  namespace fs = std::filesystem;
  if (!fs::exists(vllm_path))
    return {};

  std::vector<EngineVariantResponse> variants;
  for (const auto& entry : fs::directory_iterator(vllm_path)) {
    const auto name = "linux-amd64-cuda";  // arbitrary
    const auto version_str = "v" + entry.path().filename().string();
    const EngineVariantResponse variant{name, version_str, kVllmEngine};
    variants.push_back(variant);
  }
  return variants;
}

// NOTE: doesn't do anything
void VllmEngine::Load(EngineLoadOption opts) {
  CTL_WRN("EngineLoadOption is ignored");
  return;
};

// NOTE: doesn't do anything
void VllmEngine::Unload(EngineUnloadOption opts) {
  return;
};

// cortex.llamacpp interface
void VllmEngine::HandleChatCompletion(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};

void VllmEngine::HandleEmbedding(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};

void VllmEngine::LoadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model")) {
    auto [status, error] =
        CreateResponse("Missing required fields: model", 400);
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string model = (*json_body)["model"].asString();

  {
    std::unique_lock write_lock(mutex);
    if (model_process_map.find(model) != model_process_map.end()) {
      // check if model is still alive
      if (model_process_map[model].IsAlive()) {
        auto [status, error] = CreateResponse("Model already loaded!", 409);
        callback(std::move(status), std::move(error));
        return;
      } else {
        // if model has exited, try to load model again
        CTL_WRN("Model " << model << " has exited unexpectedly");
        model_process_map.erase(model);
      }
    }
  }

  // pid_t pid;
  // try {
  //   // https://docs.astral.sh/uv/reference/cli/#uv-run
  //   std::vector<std::string> command =
  //       python_utils::BuildUvCommand("run", model_dir.string());
  //   for (const auto& item : py_cfg.entrypoint)
  //     command.push_back(item);

  //   const std::string stdout_path = (model_dir / "stdout.txt").string();
  //   const std::string stderr_path = (model_dir / "stderr.txt").string();

  //   // create empty stdout.txt and stderr.txt for redirection
  //   if (!std::filesystem::exists(stdout_path))
  //     std::ofstream(stdout_path).flush();
  //   if (!std::filesystem::exists(stderr_path))
  //     std::ofstream(stderr_path).flush();

  //   auto result =
  //       cortex::process::SpawnProcess(command, stdout_path, stderr_path);
  //   if (result.has_error()) {
  //     throw std::runtime_error(result.error());
  //   }

  //   PythonSubprocess py_proc;
  //   py_proc.proc_info = result.value();
  //   py_proc.port = py_cfg.port;
  //   py_proc.start_time = std::chrono::system_clock::now().time_since_epoch() /
  //                        std::chrono::milliseconds(1);

  //   pid = py_proc.proc_info.pid;

  //   std::unique_lock write_lock(mutex);
  //   model_process_map[model] = py_proc;

  // } catch (const std::exception& e) {
  //   auto e_msg = e.what();
  //   auto [status, error] = CreateResponse(e_msg, k500InternalServerError);
  //   callback(std::move(status), std::move(error));
  //   return;
  // }

  // auto [status, res] = CreateResponse(
  //     "Model loaded successfully with pid: " + std::to_string(pid), k200OK);
  // callback(std::move(status), std::move(res));

  // CTL_WRN("Not implemented");
  // throw std::runtime_error("Not implemented");
};

void VllmEngine::UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};

void VllmEngine::GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};

// For backward compatible checking
bool VllmEngine::IsSupported(const std::string& f) {
  return true;
};

// Get list of running models
void VllmEngine::GetModels(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};

bool VllmEngine::SetFileLogger(int max_log_lines, const std::string& log_path) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};
void VllmEngine::SetLogLevel(trantor::Logger::LogLevel logLevel) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};

// Stop inflight chat completion in stream mode
void VllmEngine::StopInferencing(const std::string& model_id) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};
