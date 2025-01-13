#pragma once

#include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>

#include <ctime>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "utils/format_utils.h"
#include "utils/remote_models_utils.h"

namespace config {

struct RemoteModelConfig {
  std::string model;
  std::string header_template;
  std::string engine;
  std::string version;
  size_t created;
  std::string object = "model";
  std::string owned_by = "";
  Json::Value inference_params;
  Json::Value transform_req;
  Json::Value transform_resp;
  Json::Value metadata;
  void LoadFromJson(const Json::Value& json) {
    if (!json.isObject()) {
      throw std::runtime_error("Input JSON must be an object");
    }

    // Load basic string fields
    model = json.get("model", model).asString();
    header_template =
        json.get("header_template", header_template).asString();
    engine = json.get("engine", engine).asString();
    version = json.get("version", version).asString();
    created =
        json.get("created", static_cast<Json::UInt64>(created)).asUInt64();
    object = json.get("object", object).asString();
    owned_by = json.get("owned_by", owned_by).asString();

    // Load JSON object fields directly
    inference_params = json.get("inference_params", inference_params);
    transform_req = json.get("transform_req", transform_req);
    transform_resp = json.get("transform_resp", transform_resp);
    metadata = json.get("metadata", metadata);
  }

  Json::Value ToJson() const {
    Json::Value json;

    // Add basic string fields
    json["model"] = model;
    json["header_template"] = header_template;
    json["engine"] = engine;
    json["version"] = version;
    json["created"] = static_cast<Json::UInt64>(created);
    json["object"] = object;
    json["owned_by"] = owned_by;

    // Add JSON object fields directly
    json["inference_params"] = inference_params;
    json["transform_req"] = transform_req;
    json["transform_resp"] = transform_resp;
    json["metadata"] = metadata;

    return json;
  };

  void SaveToYamlFile(const std::string& filepath) const {
    YAML::Node root;

    // Convert basic fields
    root["model"] = model;
    root["header_template"] = header_template;
    root["engine"] = engine;
    root["version"] = version;
    root["object"] = object;
    root["owned_by"] = owned_by;
    root["created"] = std::time(nullptr);

    // Convert Json::Value to YAML::Node using utility function
    root["inference_params"] =
        remote_models_utils::jsonToYaml(inference_params);
    root["transform_req"] = remote_models_utils::jsonToYaml(transform_req);
    root["transform_resp"] = remote_models_utils::jsonToYaml(transform_resp);
    root["metadata"] = remote_models_utils::jsonToYaml(metadata);

    // Save to file
    std::ofstream fout(filepath);
    if (!fout.is_open()) {
      throw std::runtime_error("Failed to open file for writing: " + filepath);
    }
    fout << root;
  }

  void LoadFromYamlFile(const std::string& filepath) {
    YAML::Node root;
    try {
      root = YAML::LoadFile(filepath);
    } catch (const YAML::Exception& e) {
      throw std::runtime_error("Failed to parse YAML file: " +
                               std::string(e.what()));
    }

    // Load basic fields
    model = root["model"].as<std::string>("");
    header_template = root["header_template"].as<std::string>("");
    engine = root["engine"].as<std::string>("");
    version = root["version"] ? root["version"].as<std::string>() : "";
    created = root["created"] ? root["created"].as<std::size_t>() : 0;
    object = root["object"] ? root["object"].as<std::string>() : "model";
    owned_by = root["owned_by"] ? root["owned_by"].as<std::string>() : "";

    // Load complex fields using utility function
    inference_params =
        remote_models_utils::yamlToJson(root["inference_params"]);
    transform_req = remote_models_utils::yamlToJson(root["transform_req"]);
    transform_resp = remote_models_utils::yamlToJson(root["transform_resp"]);
    metadata = remote_models_utils::yamlToJson(root["metadata"]);
  }
};

struct ModelConfig {
  std::string name;
  std::string model;
  std::string version;
  std::vector<std::string> stop = {};
  float top_p = std::numeric_limits<float>::quiet_NaN();
  float temperature = std::numeric_limits<float>::quiet_NaN();
  float frequency_penalty = std::numeric_limits<float>::quiet_NaN();
  float presence_penalty = std::numeric_limits<float>::quiet_NaN();
  int max_tokens = std::numeric_limits<int>::quiet_NaN();
  bool stream = std::numeric_limits<bool>::quiet_NaN();
  int ngl = std::numeric_limits<int>::quiet_NaN();
  int ctx_len = std::numeric_limits<int>::quiet_NaN();
  int n_parallel = 1;
  int cpu_threads = -1;
  std::string engine;
  std::string prompt_template;
  std::string system_template;
  std::string user_template;
  std::string ai_template;

  std::string os;
  std::string gpu_arch;
  std::string quantization_method;
  std::string precision;
  int tp = std::numeric_limits<int>::quiet_NaN();
  std::string trtllm_version;
  bool text_model = std::numeric_limits<bool>::quiet_NaN();
  std::string id;
  std::vector<std::string> files;
  std::size_t created;
  std::string object;
  std::string owned_by = "";

  int seed = -1;
  float dynatemp_range = 0.0f;
  float dynatemp_exponent = 1.0f;
  int top_k = 40;
  float min_p = 0.05f;
  float tfs_z = 1.0f;
  float typ_p = 1.0f;
  int repeat_last_n = 64;
  float repeat_penalty = 1.0f;
  bool mirostat = false;
  float mirostat_tau = 5.0f;
  float mirostat_eta = 0.1f;
  bool penalize_nl = false;
  bool ignore_eos = false;
  int n_probs = 0;
  int min_keep = 0;
  uint64_t size = 0;
  std::string grammar;

  void FromJson(const Json::Value& json) {
    // do now allow to update ID and model field because it is unique identifier
    // if (json.isMember("id"))
    //   id = json["id"].asString();
    if (json.isMember("name"))
      name = json["name"].asString();
    // if (json.isMember("model"))
    //   model = json["model"].asString();
    if (json.isMember("version"))
      version = json["version"].asString();
    if (json.isMember("size"))
      size = json["size"].asUInt64();

    if (json.isMember("stop") && json["stop"].isArray()) {
      stop.clear();
      for (const auto& s : json["stop"]) {
        stop.push_back(s.asString());
      }
    }

    if (json.isMember("stream"))
      stream = json["stream"].asBool();
    if (json.isMember("top_p"))
      top_p = json["top_p"].asFloat();
    if (json.isMember("temperature"))
      temperature = json["temperature"].asFloat();
    if (json.isMember("frequency_penalty"))
      frequency_penalty = json["frequency_penalty"].asFloat();
    if (json.isMember("presence_penalty"))
      presence_penalty = json["presence_penalty"].asFloat();
    if (json.isMember("max_tokens"))
      max_tokens = json["max_tokens"].asInt();
    if (json.isMember("seed"))
      seed = json["seed"].asInt();
    if (json.isMember("dynatemp_range"))
      dynatemp_range = json["dynatemp_range"].asFloat();
    if (json.isMember("dynatemp_exponent"))
      dynatemp_exponent = json["dynatemp_exponent"].asFloat();
    if (json.isMember("top_k"))
      top_k = json["top_k"].asInt();
    if (json.isMember("min_p"))
      min_p = json["min_p"].asFloat();
    if (json.isMember("tfs_z"))
      tfs_z = json["tfs_z"].asFloat();
    if (json.isMember("typ_p"))
      typ_p = json["typ_p"].asFloat();
    if (json.isMember("repeat_last_n"))
      repeat_last_n = json["repeat_last_n"].asInt();
    if (json.isMember("repeat_penalty"))
      repeat_penalty = json["repeat_penalty"].asFloat();
    if (json.isMember("mirostat"))
      mirostat = json["mirostat"].asBool();
    if (json.isMember("mirostat_tau"))
      mirostat_tau = json["mirostat_tau"].asFloat();
    if (json.isMember("mirostat_eta"))
      mirostat_eta = json["mirostat_eta"].asFloat();
    if (json.isMember("penalize_nl"))
      penalize_nl = json["penalize_nl"].asBool();
    if (json.isMember("ignore_eos"))
      ignore_eos = json["ignore_eos"].asBool();
    if (json.isMember("n_probs"))
      n_probs = json["n_probs"].asInt();
    if (json.isMember("min_keep"))
      min_keep = json["min_keep"].asInt();
    if (json.isMember("ngl"))
      ngl = json["ngl"].asInt();
    if (json.isMember("ctx_len"))
      ctx_len = json["ctx_len"].asInt();
    if (json.isMember("n_parallel"))
      n_parallel = json["n_parallel"].asInt();
    if (json.isMember("cpu_threads"))
      cpu_threads = json["cpu_threads"].asInt();
    if (json.isMember("engine"))
      engine = json["engine"].asString();
    if (json.isMember("prompt_template"))
      prompt_template = json["prompt_template"].asString();
    if (json.isMember("system_template"))
      system_template = json["system_template"].asString();
    if (json.isMember("user_template"))
      user_template = json["user_template"].asString();
    if (json.isMember("ai_template"))
      ai_template = json["ai_template"].asString();
    if (json.isMember("os"))
      os = json["os"].asString();
    if (json.isMember("gpu_arch"))
      gpu_arch = json["gpu_arch"].asString();
    if (json.isMember("quantization_method"))
      quantization_method = json["quantization_method"].asString();
    if (json.isMember("precision"))
      precision = json["precision"].asString();

    if (json.isMember("files") && json["files"].isArray()) {
      files.clear();
      for (const auto& file : json["files"]) {
        files.push_back(file.asString());
      }
    }

    if (json.isMember("created"))
      created = json["created"].asUInt64();
    if (json.isMember("object"))
      object = json["object"].asString();
    if (json.isMember("owned_by"))
      owned_by = json["owned_by"].asString();
    if (json.isMember("text_model"))
      text_model = json["text_model"].asBool();

    if (engine == "cortex.tensorrt-llm") {
      if (json.isMember("trtllm_version"))
        trtllm_version = json["trtllm_version"].asString();
      if (json.isMember("tp"))
        tp = json["tp"].asInt();
    }
  }

  std::string ToJsonString() const {
    auto obj = ToJson();
    obj["id"] = obj["model"].asString();
    Json::StreamWriterBuilder wbuilder;
    wbuilder.settings_["precision"] = 2;
    return Json::writeString(wbuilder, obj);
  }

  Json::Value ToJson() const {
    Json::Value obj;

    obj["name"] = name;
    obj["model"] = model;
    obj["version"] = version;
    obj["size"] = size;

    Json::Value stop_array(Json::arrayValue);
    for (const auto& s : stop) {
      stop_array.append(s);
    }
    obj["stop"] = stop_array;

    obj["stream"] = stream;
    obj["top_p"] = top_p;
    obj["temperature"] = temperature;
    obj["frequency_penalty"] = frequency_penalty;
    obj["presence_penalty"] = presence_penalty;
    obj["max_tokens"] = max_tokens;
    obj["seed"] = seed;
    obj["dynatemp_range"] = dynatemp_range;
    obj["dynatemp_exponent"] = dynatemp_exponent;
    obj["top_k"] = top_k;
    obj["min_p"] = min_p;
    obj["tfs_z"] = tfs_z;
    obj["typ_p"] = typ_p;
    obj["repeat_last_n"] = repeat_last_n;
    obj["repeat_penalty"] = repeat_penalty;
    obj["mirostat"] = mirostat;
    obj["mirostat_tau"] = mirostat_tau;
    obj["mirostat_eta"] = mirostat_eta;
    obj["penalize_nl"] = penalize_nl;
    obj["ignore_eos"] = ignore_eos;
    obj["n_probs"] = n_probs;
    obj["min_keep"] = min_keep;
    obj["ngl"] = ngl;
    obj["ctx_len"] = ctx_len;
    obj["n_parallel"] = n_parallel;
    if (cpu_threads > 0) {
      obj["cpu_threads"] = cpu_threads;
    }
    obj["engine"] = engine;
    obj["prompt_template"] = prompt_template;
    obj["system_template"] = system_template;
    obj["user_template"] = user_template;
    obj["ai_template"] = ai_template;
    obj["os"] = os;
    obj["gpu_arch"] = gpu_arch;
    obj["quantization_method"] = quantization_method;
    obj["precision"] = precision;

    Json::Value files_array(Json::arrayValue);
    for (const auto& file : files) {
      files_array.append(file);
    }
    obj["files"] = files_array;

    obj["created"] = static_cast<Json::UInt64>(created);
    obj["object"] = object;
    obj["owned_by"] = owned_by;
    obj["text_model"] = text_model;

    if (engine == "cortex.tensorrt-llm") {
      obj["trtllm_version"] = trtllm_version;
      obj["tp"] = tp;
    }

    return obj;
  }
  std::string ToString() const {
    std::ostringstream oss;

    oss << format_utils::print_comment("BEGIN GENERAL GGUF METADATA");
    if (!id.empty())
      oss << format_utils::print_kv("id", id, format_utils::YELLOW);
    if (!name.empty())
      oss << format_utils::print_kv("name", name, format_utils::YELLOW);
    if (!model.empty())
      oss << format_utils::print_kv("model", model, format_utils::YELLOW);
    if (!version.empty())
      oss << format_utils::print_kv("version", version, format_utils::YELLOW);
    if (!files.empty()) {
      oss << format_utils::GREEN << "files:" << format_utils::RESET << "\n";
      for (const auto& file : files) {
        oss << "  - " << format_utils::YELLOW << file << format_utils::RESET
            << "\n";
      }
    }
    oss << format_utils::print_comment("END GENERAL GGUF METADATA");

    oss << format_utils::print_comment("BEGIN INFERENCE PARAMETERS");
    oss << format_utils::print_comment("BEGIN REQUIRED");
    if (!stop.empty()) {
      oss << format_utils::GREEN << "stop:" << format_utils::RESET << "\n";
      for (const auto& s : stop) {
        oss << "  - " << format_utils::YELLOW << s << format_utils::RESET
            << "\n";
      }
    }
    oss << format_utils::print_comment("END REQUIRED");
    oss << format_utils::print_comment("BEGIN OPTIONAL");

    oss << format_utils::print_float("size", size);
    oss << format_utils::print_bool("stream", stream);
    oss << format_utils::print_float("top_p", top_p);
    oss << format_utils::print_float("temperature", temperature);
    oss << format_utils::print_float("frequency_penalty", frequency_penalty);
    oss << format_utils::print_float("presence_penalty", presence_penalty);
    if (max_tokens != std::numeric_limits<int>::quiet_NaN())
      oss << format_utils::print_kv("max_tokens", std::to_string(max_tokens),
                                    format_utils::MAGENTA);
    if (seed != -1)
      oss << format_utils::print_kv("seed", std::to_string(seed),
                                    format_utils::MAGENTA);
    oss << format_utils::print_float("dynatemp_range", dynatemp_range);
    oss << format_utils::print_float("dynatemp_exponent", dynatemp_exponent);
    oss << format_utils::print_kv("top_k", std::to_string(top_k),
                                  format_utils::MAGENTA);
    oss << format_utils::print_float("min_p", min_p);
    oss << format_utils::print_float("tfs_z", tfs_z);
    oss << format_utils::print_float("typ_p", typ_p);
    oss << format_utils::print_kv(
        "repeat_last_n", std::to_string(repeat_last_n), format_utils::MAGENTA);
    oss << format_utils::print_float("repeat_penalty", repeat_penalty);
    oss << format_utils::print_bool("mirostat", mirostat);
    oss << format_utils::print_float("mirostat_tau", mirostat_tau);
    oss << format_utils::print_float("mirostat_eta", mirostat_eta);
    oss << format_utils::print_bool("penalize_nl", penalize_nl);
    oss << format_utils::print_bool("ignore_eos", ignore_eos);
    oss << format_utils::print_kv("n_probs", std::to_string(n_probs),
                                  format_utils::MAGENTA);
    oss << format_utils::print_kv("min_keep", std::to_string(min_keep),
                                  format_utils::MAGENTA);

    oss << format_utils::print_comment("END OPTIONAL");
    oss << format_utils::print_comment("END INFERENCE PARAMETERS");
    oss << format_utils::print_comment("BEGIN MODEL LOAD PARAMETERS");
    oss << format_utils::print_comment("BEGIN REQUIRED");

    if (!engine.empty())
      oss << format_utils::print_kv("engine", engine, format_utils::YELLOW);
    if (!prompt_template.empty())
      oss << format_utils::print_kv("prompt_template", prompt_template,
                                    format_utils::YELLOW);

    oss << format_utils::print_comment("END REQUIRED");
    oss << format_utils::print_comment("BEGIN OPTIONAL");

    if (ctx_len != std::numeric_limits<int>::quiet_NaN())
      oss << format_utils::print_kv("ctx_len", std::to_string(ctx_len),
                                    format_utils::MAGENTA);
    oss << format_utils::print_kv("n_parallel", std::to_string(n_parallel),
                                  format_utils::MAGENTA);
    oss << format_utils::print_kv("cpu_threads", std::to_string(cpu_threads),
                                  format_utils::MAGENTA);
    if (ngl != std::numeric_limits<int>::quiet_NaN())
      oss << format_utils::print_kv("ngl", std::to_string(ngl),
                                    format_utils::MAGENTA);

    oss << format_utils::print_comment("END OPTIONAL");
    oss << format_utils::print_comment("END MODEL LOAD PARAMETERS");

    return oss.str();
  }
};

struct Endpoint {
  std::string method;
  std::string path;
  std::string transform_request;
  std::string transform_response;
};

struct PythonModelConfig {
  // General Metadata
  std::string id;
  std::string model;
  std::string name;
  int version;

  // Inference Parameters
  Endpoint load_model;
  Endpoint destroy;
  Endpoint inference;
  Endpoint heath_check;
  std::vector<Endpoint> extra_endpoints;

  // Model Load Parameters
  std::string port;
  std::string script;
  std::string log_path;
  std::string log_level;
  std::string environment;
  std::vector<std::string> command;  // New command field
  std::vector<std::string> files;
  std::vector<std::string> depends;
  std::string engine;
  Json::Value extra_params;  // Accept dynamic extra parameters

  // Method to convert C++ struct to YAML
  void ToYaml(const std::string& filepath) const {
    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "id" << YAML::Value << id;
    out << YAML::Key << "model" << YAML::Value << model;
    out << YAML::Key << "name" << YAML::Value << name;
    out << YAML::Key << "version" << YAML::Value << version;

    // Inference Parameters
    out << YAML::Key << "load_model" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "method" << YAML::Value << load_model.method;
    out << YAML::Key << "path" << YAML::Value << load_model.path;
    out << YAML::Key << "transform_request" << YAML::Value
        << load_model.transform_request;
    out << YAML::Key << "transform_response" << YAML::Value
        << load_model.transform_response;
    out << YAML::EndMap;

    out << YAML::Key << "destroy" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "method" << YAML::Value << destroy.method;
    out << YAML::Key << "path" << YAML::Value << destroy.path;
    out << YAML::EndMap;

    out << YAML::Key << "inference" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "method" << YAML::Value << inference.method;
    out << YAML::Key << "path" << YAML::Value << inference.path;
    out << YAML::EndMap;

    out << YAML::Key << "extra_endpoints" << YAML::Value << YAML::BeginSeq;
    for (const auto& endpoint : extra_endpoints) {
      out << YAML::BeginMap;
      out << YAML::Key << "method" << YAML::Value << endpoint.method;
      out << YAML::Key << "path" << YAML::Value << endpoint.path;
      out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    // Model Load Parameters
    out << YAML::Key << "port" << YAML::Value << port;
    out << YAML::Key << "script" << YAML::Value << script;
    out << YAML::Key << "log_path" << YAML::Value << log_path;
    out << YAML::Key << "log_level" << YAML::Value << log_level;
    out << YAML::Key << "environment" << YAML::Value << environment;

    // Serialize command as YAML list
    out << YAML::Key << "command" << YAML::Value << YAML::BeginSeq;
    for (const auto& cmd : command) {
      out << cmd;
    }
    out << YAML::EndSeq;

    // Serialize files as YAML list
    out << YAML::Key << "files" << YAML::Value << YAML::BeginSeq;
    for (const auto& file : files) {
      out << file;
    }
    out << YAML::EndSeq;

    // Serialize command as YAML list
    out << YAML::Key << "depends" << YAML::Value << YAML::BeginSeq;
    for (const auto& depend : depends) {
      out << depend;
    }
    out << YAML::EndSeq;

    out << YAML::Key << "engine" << YAML::Value << engine;

    // Serialize extra_params as YAML
    out << YAML::Key << "extra_params" << YAML::Value << YAML::BeginMap;
    for (Json::ValueConstIterator iter = extra_params.begin();
         iter != extra_params.end(); ++iter) {
      out << YAML::Key << iter.key().asString() << YAML::Value
          << iter->asString();
    }
    out << YAML::EndMap;

    std::ofstream fout(filepath);
    if (!fout.is_open()) {
      throw std::runtime_error("Failed to open file for writing: " + filepath);
    }
    fout << out.c_str();
  }

  // Method to populate struct from YAML file
  void ReadFromYaml(const std::string& filePath) {
    YAML::Node config = YAML::LoadFile(filePath);

    if (config["id"])
      id = config["id"].as<std::string>();
    if (config["model"])
      model = config["model"].as<std::string>();
    if (config["name"])
      name = config["name"].as<std::string>();
    if (config["version"])
      version = config["version"].as<int>();

    // Inference Parameters

    auto ip = config;
    if (ip["load_model"]) {
      load_model.method = ip["load_model"]["method"].as<std::string>();
      load_model.path = ip["load_model"]["path"].as<std::string>();
      load_model.transform_request =
          ip["load_model"]["transform_request"].as<std::string>();
      load_model.transform_response =
          ip["load_model"]["transform_response"].as<std::string>();
    }
    if (ip["destroy"]) {
      destroy.method = ip["destroy"]["method"].as<std::string>();
      destroy.path = ip["destroy"]["path"].as<std::string>();
    }
    if (ip["inference"]) {
      inference.method = ip["inference"]["method"].as<std::string>();
      inference.path = ip["inference"]["path"].as<std::string>();
    }
    if (ip["extra_endpoints"] && ip["extra_endpoints"].IsSequence()) {
      for (const auto& endpoint : ip["extra_endpoints"]) {
        Endpoint e;
        e.method = endpoint["method"].as<std::string>();
        e.path = endpoint["path"].as<std::string>();
        extra_endpoints.push_back(e);
      }
    }

    // Model Load Parameters

    auto mlp = config;
    if (mlp["port"])
      port = mlp["port"].as<std::string>();
    if (mlp["script"])
      script = mlp["script"].as<std::string>();
    if (mlp["log_path"])
      log_path = mlp["log_path"].as<std::string>();
    if (mlp["log_level"])
      log_level = mlp["log_level"].as<std::string>();
    if (mlp["environment"])
      environment = mlp["environment"].as<std::string>();
    if (mlp["engine"])
      engine = mlp["engine"].as<std::string>();

    if (mlp["command"] && mlp["command"].IsSequence()) {
      for (const auto& cmd : mlp["command"]) {
        command.push_back(cmd.as<std::string>());
      }
    }

    if (mlp["files"] && mlp["files"].IsSequence()) {
      for (const auto& file : mlp["files"]) {
        files.push_back(file.as<std::string>());
      }
    }

    if (mlp["depends"] && mlp["depends"].IsSequence()) {
      for (const auto& depend : mlp["depends"]) {
        depends.push_back(depend.as<std::string>());
      }
    }

    if (mlp["extra_params"]) {
      for (YAML::const_iterator it = mlp["extra_params"].begin();
           it != mlp["extra_params"].end(); ++it) {
        extra_params[it->first.as<std::string>()] =
            it->second.as<std::string>();
      }
    }
  }

  // Method to convert the struct to JSON
  Json::Value ToJson() const {
    Json::Value root;

    root["id"] = id;
    root["model"] = model;
    root["name"] = name;
    root["version"] = version;

    // Inference Parameters
    root["load_model"]["method"] = load_model.method;
    root["load_model"]["path"] = load_model.path;
    root["load_model"]["transform_request"] = load_model.transform_request;
    root["load_model"]["transform_response"] = load_model.transform_response;

    root["destroy"]["method"] = destroy.method;
    root["destroy"]["path"] = destroy.path;

    root["inference"]["method"] = inference.method;
    root["inference"]["path"] = inference.path;

    for (const auto& endpoint : extra_endpoints) {
      Json::Value e;
      e["method"] = endpoint.method;
      e["path"] = endpoint.path;
      root["extra_endpoints"].append(e);
    }

    // Model Load Parameters
    root["port"] = port;
    root["log_path"] = log_path;
    root["log_level"] = log_level;
    root["environment"] = environment;
    root["script"] = script;

    // Serialize command as JSON array
    for (const auto& cmd : command) {
      root["command"].append(cmd);
    }

    for (const auto& file : files) {
      root["files"].append(file);
    }

    for (const auto& depend : depends) {
      root["depends"].append(depend);
    }

    root["engine"] = engine;
    root["extra_params"] = extra_params;  // Serialize the JSON value directly

    return root;
  }

  // Method to populate struct from JSON
  void FromJson(const Json::Value& root) {

    if (root.isMember("id"))
      id = root["id"].asString();
    if (root.isMember("model"))
      model = root["model"].asString();
    if (root.isMember("name"))
      name = root["name"].asString();
    if (root.isMember("version"))
      version = root["version"].asInt();

    // Inference Parameters

    const Json::Value& ip = root;
    if (ip.isMember("load_model")) {
      load_model.method = ip["load_model"]["method"].asString();
      load_model.path = ip["load_model"]["path"].asString();
      load_model.transform_request =
          ip["load_model"]["transform_request"].asString();
      load_model.transform_response =
          ip["load_model"]["transform_response"].asString();
    }
    if (ip.isMember("destroy")) {
      destroy.method = ip["destroy"]["method"].asString();
      destroy.path = ip["destroy"]["path"].asString();
    }
    if (ip.isMember("inference")) {
      inference.method = ip["inference"]["method"].asString();
      inference.path = ip["inference"]["path"].asString();
    }
    if (ip.isMember("extra_endpoints")) {
      for (const auto& endpoint : ip["extra_endpoints"]) {
        Endpoint e;
        e.method = endpoint["method"].asString();
        e.path = endpoint["path"].asString();
        extra_endpoints.push_back(e);
      }
    }

    // Model Load Parameters

    const Json::Value& mlp = root;
    if (mlp.isMember("port"))
      port = mlp["port"].asString();
    if (mlp.isMember("log_path"))
      log_path = mlp["log_path"].asString();
    if (mlp.isMember("log_level"))
      log_level = mlp["log_level"].asString();
    if (mlp.isMember("environment"))
      environment = mlp["environment"].asString();
    if (mlp.isMember("engine"))
      engine = mlp["engine"].asString();
    if (mlp.isMember("script"))
      script = mlp["script"].asString();

    if (mlp.isMember("command")) {
      for (const auto& cmd : mlp["command"]) {
        command.push_back(cmd.asString());
      }
    }

    if (mlp.isMember("files")) {
      for (const auto& file : mlp["files"]) {
        files.push_back(file.asString());
      }
    }

    if (mlp.isMember("depends")) {
      for (const auto& depend : mlp["depends"]) {
        depends.push_back(depend.asString());
      }
    }

    if (mlp.isMember("extra_params")) {
      extra_params = mlp["extra_params"];  // Directly assign the JSON value
    }
  }
};

}  // namespace config
