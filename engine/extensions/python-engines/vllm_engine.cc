#include "vllm_engine.h"
#include <fstream>
#include "services/engine_service.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/system_info_utils.h"

namespace {
static std::pair<Json::Value, Json::Value> CreateResponse(
    const std::string& msg, int code) {
  Json::Value status, res;
  status["status_code"] = code;
  status["has_error"] = code != 200;
  res["message"] = msg;
  return {status, res};
}
}  // namespace

VllmEngine::VllmEngine()
    : cortex_port_{std::stoi(
          file_manager_utils::GetCortexConfig().apiServerPort)},
      port_offsets_{true}  // cortex_port + 0 is always used (by cortex itself)
{}

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

void VllmEngine::Load(EngineLoadOption opts) {
  version_ = opts.engine_path;  // engine path actually contains version info
  if (version_[0] == 'v')
    version_ = version_.substr(1);
  return;
};

void VllmEngine::Unload(EngineUnloadOption opts) {};

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
      auto proc = model_process_map[model];

      if (proc.IsAlive()) {
        auto [status, error] = CreateResponse("Model already loaded!", 409);
        callback(std::move(status), std::move(error));
        return;
      } else {
        // if model has exited, try to load model again?
        CTL_WRN("Model " << model << " has exited unexpectedly");
        model_process_map.erase(model);
        port_offsets_[proc.port - cortex_port_] = false;  // free the port
      }
    }
  }

  pid_t pid;
  try {
    namespace fs = std::filesystem;

    const auto model_path = file_manager_utils::GetCortexDataPath() / "models" /
                            kHuggingFaceHost / model;

    auto env_dir = python_utils::GetEnvsPath() / "vllm" / version_;
    if (!fs::exists(env_dir))
      throw std::runtime_error(env_dir.string() + " does not exist");

    int offset = 1;
    for (;; offset++) {
      // add this guard to prevent endless loop
      if (offset >= 100)
        throw std::runtime_error("Unable to find an available port");

      if (port_offsets_.size() <= offset)
        port_offsets_.push_back(false);

      // check if port is used
      if (!port_offsets_[offset])
        break;
    }
    const int port = cortex_port_ + offset;

    // https://docs.astral.sh/uv/reference/cli/#uv-run
    std::vector<std::string> cmd =
        python_utils::BuildUvCommand("run", env_dir.string());
    cmd.push_back("vllm");
    cmd.push_back("serve");
    cmd.push_back(model_path.string());
    cmd.push_back("--port");
    cmd.push_back(std::to_string(port));
    cmd.push_back("--served-model-name");
    cmd.push_back(model);

    const auto stdout_file = env_dir / "stdout.log";
    const auto stderr_file = env_dir / "stderr.log";

    // create empty files for redirection
    // TODO: add limit on file size?
    if (!std::filesystem::exists(stdout_file))
      std::ofstream(stdout_file).flush();
    if (!std::filesystem::exists(stderr_file))
      std::ofstream(stderr_file).flush();

    // TODO: may want to wait until model is ready i.e. health check endpoint
    auto result = cortex::process::SpawnProcess(cmd, stdout_file.string(),
                                                stderr_file.string());
    if (result.has_error()) {
      throw std::runtime_error(result.error());
    }

    python_utils::PythonSubprocess py_proc;
    py_proc.proc_info = result.value();
    py_proc.port = port;
    py_proc.start_time = std::chrono::system_clock::now().time_since_epoch() /
                         std::chrono::milliseconds(1);

    pid = py_proc.proc_info.pid;

    std::unique_lock write_lock(mutex);
    model_process_map[model] = py_proc;

  } catch (const std::exception& e) {
    auto e_msg = e.what();
    auto [status, error] = CreateResponse(e_msg, 500);
    callback(std::move(status), std::move(error));
    return;
  }

  auto [status, res] = CreateResponse(
      "Model loaded successfully with pid: " + std::to_string(pid), 200);
  callback(std::move(status), std::move(res));
};

void VllmEngine::UnloadModel(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  if (!json_body->isMember("model")) {
    auto [status, error] = CreateResponse("Missing required field: model", 400);
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string model = (*json_body)["model"].asString();

  // check if model has started
  {
    std::shared_lock read_lock(mutex);
    if (model_process_map.find(model) == model_process_map.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  // we know that model has started
  {
    std::unique_lock write_lock(mutex);
    auto proc = model_process_map[model];

    // TODO: we can use vLLM health check endpoint
    // check if subprocess is still alive
    // NOTE: is this step necessary? the subprocess could have terminated
    // after .IsAlive() and before .Kill() later.
    if (!proc.IsAlive()) {
      model_process_map.erase(model);
      port_offsets_[proc.port - cortex_port_] = false;  // free the port

      const std::string msg = "Model " + model + " stopped running.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }

    // subprocess is alive. we kill it here.
    if (!model_process_map[model].Kill()) {
      const std::string msg = "Unable to kill process of model " + model;
      auto [status, error] = CreateResponse(msg, 500);
      callback(std::move(status), std::move(error));
      return;
    }

    model_process_map.erase(model);
    port_offsets_[proc.port - cortex_port_] = false;  // free the port
  }

  auto [status, res] = CreateResponse("Unload model successfully", 200);
  callback(std::move(status), std::move(res));
};

void VllmEngine::GetModelStatus(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model")) {
    auto [status, error] = CreateResponse("Missing required field: model", 400);
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string model = (*json_body)["model"].asString();
  // check if model has started
  {
    std::shared_lock read_lock(mutex);
    if (model_process_map.find(model) == model_process_map.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  // we know that model has started
  // TODO: just use health check endpoint
  {
    std::unique_lock write_lock(mutex);

    // check if subprocess is still alive
    if (!model_process_map[model].IsAlive()) {
      CTL_WRN("Model " << model << " has exited unexpectedly.");
      model_process_map.erase(model);
      const std::string msg = "Model " + model + " stopped running.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  Json::Value res, status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = 200;
  callback(std::move(status), std::move(res));
};

bool VllmEngine::IsSupported(const std::string& f) {
  return true;
};

void VllmEngine::GetModels(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  Json::Value res, model_list(Json::arrayValue), status;
  {
    std::unique_lock write_lock(mutex);
    for (auto& [model_name, py_proc] : model_process_map) {
      // TODO: check using health endpoint
      if (!py_proc.IsAlive()) {
        CTL_WRN("Model " << model_name << " has exited unexpectedly.");
        model_process_map.erase(model_name);
        continue;
      }

      Json::Value val;
      val["id"] = model_name;
      val["engine"] = kVllmEngine;
      val["start_time"] = py_proc.start_time;
      val["port"] = py_proc.port;
      val["object"] = "model";
      // TODO
      // val["ram"];
      // val["vram"];
      model_list.append(val);
    }
  }

  res["object"] = "list";
  res["data"] = model_list;

  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = 200;

  callback(std::move(status), std::move(res));
};

bool VllmEngine::SetFileLogger(int max_log_lines, const std::string& log_path) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};
void VllmEngine::SetLogLevel(trantor::Logger::LogLevel logLevel) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};

void VllmEngine::StopInferencing(const std::string& model_id) {
  CTL_WRN("Not implemented");
  throw std::runtime_error("Not implemented");
};
