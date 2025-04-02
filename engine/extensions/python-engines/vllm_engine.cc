// Note on subprocess lifecycle
// In LoadModel(), we will wait until /health returns 200. Thus, in subsequent
// calls to the subprocess, if the server is working normally, /health is
// guaranteed to return 200. If it doesn't, it either means the subprocess has
// died or the server hangs (for whatever reason).

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

// this is mostly copied from local_engine.cc
struct StreamContext {
  std::shared_ptr<std::function<void(Json::Value&&, Json::Value&&)>> callback;
  bool need_stop;

  static size_t write_callback(char* ptr, size_t size, size_t nmemb,
                               void* userdata) {
    auto* ctx = static_cast<StreamContext*>(userdata);
    size_t data_length = size * nmemb;
    if (data_length <= 6)
      return data_length;

    std::string chunk{ptr, data_length};
    CTL_INF(chunk);
    Json::Value status;
    status["is_stream"] = true;
    status["has_error"] = false;
    status["status_code"] = 200;
    Json::Value chunk_json;
    chunk_json["data"] = chunk;

    if (chunk.find("[DONE]") != std::string::npos) {
      status["is_done"] = true;
      ctx->need_stop = false;
    } else {
      status["is_done"] = false;
    }

    (*ctx->callback)(std::move(status), std::move(chunk_json));
    return data_length;
  };
};

}  // namespace

VllmEngine::VllmEngine()
    : cortex_port_{std::stoi(
          file_manager_utils::GetCortexConfig().apiServerPort)},
      port_offsets_{true},  // cortex_port + 0 is always used (by cortex itself)
      queue_{2 /* threadNum */, "vLLM engine"} {}

VllmEngine::~VllmEngine() {
  // NOTE: what happens if we can't kill subprocess?
  std::unique_lock write_lock(mutex_);
  for (auto& [model_name, py_proc] : model_process_map_) {
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
    // TODO: after llama-server is merged, check if we need to add "v"
    const auto version_str = "v" + entry.path().filename().string();
    const EngineVariantResponse variant{name, version_str, kVllmEngine};
    variants.push_back(variant);
  }
  return variants;
}

// TODO: once llama-server is merged, check if checking 'v' is still needed
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

  // NOTE: request validation should be in controller
  if (!json_body->isMember("model")) {
    auto [status, error] =
        CreateResponse("Missing required fields: model", 400);
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string model = (*json_body)["model"].asString();
  int port;
  // check if model has started
  {
    std::shared_lock read_lock(mutex_);
    if (model_process_map_.find(model) == model_process_map_.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }
    port = model_process_map_[model].port;
  }

  const std::string url =
      "http://127.0.0.1:" + std::to_string(port) + "/v1/chat/completions";
  const std::string json_str = json_body->toStyledString();

  bool stream = (*json_body)["stream"].asBool();
  if (stream) {
    queue_.runTaskInQueue([url = std::move(url), json_str = std::move(json_str),
                           callback = std::move(callback)] {
      CURL* curl = curl_easy_init();
      if (!curl) {
        auto [status, res] = CreateResponse("Internal server error", 500);
        callback(std::move(status), std::move(res));
      }

      struct curl_slist* headers = nullptr;
      headers = curl_slist_append(headers, "Content-Type: application/json");

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_str.length());
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

      StreamContext ctx;
      ctx.callback =
          std::make_shared<std::function<void(Json::Value&&, Json::Value&&)>>(
              callback);
      ctx.need_stop = true;
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                       StreamContext::write_callback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);

      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        auto msg = curl_easy_strerror(res);
        auto [status, res] = CreateResponse(msg, 500);
        callback(std::move(status), std::move(res));
      }

      curl_slist_free_all(headers);
      curl_easy_cleanup(curl);
      if (ctx.need_stop) {
        Json::Value status;
        status["is_done"] = true;
        status["has_error"] = false;
        status["is_stream"] = true;
        status["status_code"] = 200;
        callback(std::move(status), Json::Value{});
      }

      return;
    });
  } else {
    // non-streaming
    auto result = curl_utils::SimplePostJson(url, json_str);

    if (result.has_error()) {
      auto [status, res] = CreateResponse(result.error(), 400);
      callback(std::move(status), std::move(res));
    }

    Json::Value status;
    status["is_done"] = true;
    status["has_error"] = false;
    status["is_stream"] = false;
    status["status_code"] = 200;
    callback(std::move(status), std::move(result.value()));
  }
};

// NOTE: we don't have an option to pass --task embed to vLLM spawn yet
void VllmEngine::HandleEmbedding(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {

  if (!json_body->isMember("model")) {
    auto [status, error] =
        CreateResponse("Missing required fields: model", 400);
    callback(std::move(status), std::move(error));
    return;
  }

  const std::string model = (*json_body)["model"].asString();
  int port;
  // check if model has started
  {
    std::shared_lock read_lock(mutex_);
    if (model_process_map_.find(model) == model_process_map_.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }
    port = model_process_map_[model].port;
  }

  const std::string url =
      "http://127.0.0.1:" + std::to_string(port) + "/v1/embeddings";
  const std::string json_str = json_body->toStyledString();

  auto result = curl_utils::SimplePostJson(url, json_str);

  if (result.has_error()) {
    auto [status, res] = CreateResponse(result.error(), 400);
    callback(std::move(status), std::move(res));
  }

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = 200;
  callback(std::move(status), std::move(result.value()));
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
    std::unique_lock write_lock(mutex_);
    if (model_process_map_.find(model) != model_process_map_.end()) {
      auto proc = model_process_map_[model];

      // NOTE: each vLLM instance can only serve 1 task. It means that the
      // following logic will not allow serving the same model for 2 different
      // tasks at the same time.
      // To support it, we also need to know how vLLM decides the default task.
      if (proc.IsAlive()) {
        auto [status, error] = CreateResponse("Model already loaded!", 409);
        callback(std::move(status), std::move(error));
        return;
      } else {
        // if model has exited, try to load model again?
        CTL_WRN("Model " << model << " has exited unexpectedly");
        model_process_map_.erase(model);
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
        python_utils::UvBuildCommand("run", env_dir.string());
    cmd.push_back("vllm");
    cmd.push_back("serve");
    cmd.push_back(model_path.string());
    cmd.push_back("--port");
    cmd.push_back(std::to_string(port));
    cmd.push_back("--served-model-name");
    cmd.push_back(model);

    // NOTE: we might want to adjust max-model-len automatically, since vLLM
    // may OOM for large models as it tries to allocate full context length.
    const std::string EXTRA_ARGS[] = {"task", "max-model-len"};
    for (const auto arg : EXTRA_ARGS) {
      if (json_body->isMember(arg)) {
        cmd.push_back("--" + arg);
        cmd.push_back((*json_body)[arg].asString());
      }
    }

    const auto stdout_file = env_dir / "stdout.log";
    const auto stderr_file = env_dir / "stderr.log";

    // create empty files for redirection
    // TODO: add limit on file size?
    if (!std::filesystem::exists(stdout_file))
      std::ofstream(stdout_file).flush();
    if (!std::filesystem::exists(stderr_file))
      std::ofstream(stderr_file).flush();

    auto result = cortex::process::SpawnProcess(cmd, stdout_file.string(),
                                                stderr_file.string());
    if (result.has_error()) {
      throw std::runtime_error(result.error());
    }
    auto proc_info = result.value();
    pid = proc_info.pid;

    // wait for server to be up
    // NOTE: should we add a timeout to avoid endless loop?
    while (true) {
      CTL_INF("Wait for vLLM server to be up. Sleep for 5s");
      std::this_thread::sleep_for(std::chrono::seconds(5));
      if (!cortex::process::IsProcessAlive(proc_info))
        throw std::runtime_error("vLLM subprocess fails to start");

      const auto url = "http://127.0.0.1:" + std::to_string(port) + "/health";
      if (curl_utils::SimpleGet(url).has_value())
        break;
    }

    python_utils::PythonSubprocess py_proc;
    py_proc.proc_info = proc_info;
    py_proc.port = port;
    py_proc.start_time = std::chrono::system_clock::now().time_since_epoch() /
                         std::chrono::milliseconds(1);

    std::unique_lock write_lock(mutex_);
    model_process_map_[model] = py_proc;

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
    std::shared_lock read_lock(mutex_);
    if (model_process_map_.find(model) == model_process_map_.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  // we know that model has started
  {
    std::unique_lock write_lock(mutex_);
    auto proc = model_process_map_[model];

    // check if subprocess is still alive
    // NOTE: is this step necessary? the subprocess could have terminated
    // after .IsAlive() and before .Kill() later.
    if (!proc.IsAlive()) {
      model_process_map_.erase(model);
      port_offsets_[proc.port - cortex_port_] = false;  // free the port

      const std::string msg = "Model " + model + " stopped running.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }

    // subprocess is alive. we kill it here.
    if (!model_process_map_[model].Kill()) {
      const std::string msg = "Unable to kill process of model " + model;
      auto [status, error] = CreateResponse(msg, 500);
      callback(std::move(status), std::move(error));
      return;
    }

    model_process_map_.erase(model);
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
    std::shared_lock read_lock(mutex_);
    if (model_process_map_.find(model) == model_process_map_.end()) {
      const std::string msg = "Model " + model + " has not been loaded yet.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
      return;
    }
  }

  // we know that model has started
  {
    std::unique_lock write_lock(mutex_);
    auto py_proc = model_process_map_[model];

    // health check endpoint
    const auto url =
        "http://127.0.0.1:" + std::to_string(py_proc.port) + "/health";
    if (curl_utils::SimpleGet(url).has_value()) {
      Json::Value status;
      status["is_done"] = true;
      status["has_error"] = false;
      status["is_stream"] = false;
      status["status_code"] = 200;
      callback(std::move(status), Json::Value{});
    } else {
      // try to kill the subprocess to free resources, in case the server hangs
      // instead of subprocess has died.
      py_proc.Kill();

      CTL_WRN("Model " << model << " has exited unexpectedly.");
      model_process_map_.erase(model);
      const std::string msg = "Model " + model + " stopped running.";
      auto [status, error] = CreateResponse(msg, 400);
      callback(std::move(status), std::move(error));
    }
  }
};

bool VllmEngine::IsSupported(const std::string& f) {
  return true;
};

void VllmEngine::GetModels(
    std::shared_ptr<Json::Value> jsonBody,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  Json::Value res, model_list(Json::arrayValue), status;
  {
    std::unique_lock write_lock(mutex_);
    for (auto& [model_name, py_proc] : model_process_map_) {
      const auto url =
          "http://127.0.0.1:" + std::to_string(py_proc.port) + "/health";
      if (curl_utils::SimpleGet(url).has_error()) {
        // try to kill the subprocess to free resources, in case the server hangs
        // instead of subprocess has died.
        py_proc.Kill();

        CTL_WRN("Model " << model_name << " has exited unexpectedly.");
        model_process_map_.erase(model_name);
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
