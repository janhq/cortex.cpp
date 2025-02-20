#include "model_upd_cmd.h"
#include "server_start_cmd.h"
#include "utils/curl_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {

ModelUpdCmd::ModelUpdCmd(std::string model_handle)
    : model_handle_(std::move(model_handle)) {}

void ModelUpdCmd::Exec(
    const std::string& host, int port,
    const std::unordered_map<std::string, std::string>& options) {
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  auto url = url_parser::Url{
      .protocol = "http",
      .host = host + ":" + std::to_string(port),
      .pathParams = {"v1", "models", model_handle_},
  };

  Json::Value json_data;
  for (const auto& [key, value] : options) {
    if (!value.empty()) {
      UpdateConfig(json_data, key, value);
    }
  }
  auto data_str = json_data.toStyledString();
  auto res = curl_utils::SimplePatchJson(url.ToFullPath(), data_str);
  if (res.has_error()) {
    auto root = json_helper::ParseJsonString(res.error());
    CLI_LOG(root["message"].asString());
    return;
  }

  CLI_LOG("Successfully updated model ID '" + model_handle_ + "'!");
  return;
}

void ModelUpdCmd::UpdateConfig(Json::Value& data, const std::string& key,
                               const std::string& value) {
  static const std::unordered_map<
      std::string,
      std::function<void(Json::Value &, const std::string&, const std::string&)>>
      updaters = {
          {"name",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["name"] = v;
           }},
          {"model",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["model"] = v;
           }},
          {"version",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["version"] = v;
           }},
          {"engine",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["engine"] = v;
           }},
          {"prompt_template",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["prompt_template"] = v;
           }},
          {"system_template",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["system_template"] = v;
           }},
          {"user_template",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["user_template"] = v;
           }},
          {"ai_template",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["ai_template"] = v;
           }},
          {"os",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["os"] = v;
           }},
          {"gpu_arch",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["gpu_arch"] = v;
           }},
          {"quantization_method",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["quantization_method"] = v;
           }},
          {"precision",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["precision"] = v;
           }},
          {"trtllm_version",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["trtllm_version"] = v;
           }},
          {"object",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["object"] = v;
           }},
          {"owned_by",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["owned_by"] = v;
           }},
          {"grammar",
           [](Json::Value &data, const std::string&, const std::string& v) {
             data["grammar"] = v;
           }},
          {"stop", [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateVectorField(
                 k, v, [&data](const std::vector<std::string>& stops) { 
                  Json::Value d(Json::arrayValue);
                  for (auto const& s: stops) {
                    d.append(s);
                  }
                  data["stop"] = d; 
                  });
           }},
          {"files", [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateVectorField(
                 k, v, [&data](const std::vector<std::string>& fs) { 
                  Json::Value d(Json::arrayValue);
                  for (auto const& f: fs) {
                    d.append(f);
                  }
                  data["files"] = d; 
                  });
           }},
          {"top_p",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(
                 k, v, [&data](float f) { data["top_p"] = f; });
           }},
          {"temperature",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["temperature"] = f;
             });
           }},
          {"frequency_penalty",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["frequency_penalty"] = f;
             });
           }},
          {"presence_penalty",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["presence_penalty"] = f;
             });
           }},
          {"dynatemp_range",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["dynatemp_range"] = f;
             });
           }},
          {"dynatemp_exponent",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["dynatemp_exponent"] = f;
             });
           }},
          {"min_p",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(
                 k, v, [&data](float f) { data["min_p"] = f; });
           }},
          {"tfs_z",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(
                 k, v, [&data](float f) { data["tfs_z"] = f; });
           }},
          {"typ_p",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(
                 k, v, [&data](float f) { data["typ_p"] = f; });
           }},
          {"repeat_penalty",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["repeat_penalty"] = f;
             });
           }},
          {"mirostat_tau",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["mirostat_tau"] = f;
             });
           }},
          {"mirostat_eta",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["mirostat_eta"] = f;
             });
           }},
          {"max_tokens",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["max_tokens"] = static_cast<int>(f);
             });
           }},
          {"ngl",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["ngl"] = static_cast<int>(f);
             });
           }},
          {"ctx_len",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["ctx_len"] = static_cast<int>(f);
             });
           }},
           {"n_parallel",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["n_parallel"] = static_cast<int>(f);
             });
           }},
           {"cpu_threads",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["cpu_threads"] = static_cast<int>(f);
             });
           }},
          {"tp",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["tp"] = static_cast<int>(f);
             });
           }},
          {"seed",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["seed"] = static_cast<int>(f);
             });
           }},
          {"top_k",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["top_k"] = static_cast<int>(f);
             });
           }},
          {"repeat_last_n",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["repeat_last_n"] = static_cast<int>(f);
             });
           }},
          {"n_probs",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["n_probs"] = static_cast<int>(f);
             });
           }},
          {"min_keep",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["min_keep"] = static_cast<int>(f);
             });
           }},
          {"stream",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateBooleanField(
                 k, v, [&data](bool b) { data["stream"] = b; });
           }},
          {"text_model",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateBooleanField(
                 k, v, [&data](bool b) { data["text_model"] = b; });
           }},
          {"mirostat",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateBooleanField(
                 k, v, [&data](bool b) { data["mirostat"] = b; });
           }},
          {"penalize_nl",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateBooleanField(
                 k, v, [&data](bool b) { data["penalize_nl"] = b; });
           }},
          {"ignore_eos",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateBooleanField(
                 k, v, [&data](bool b) { data["ignore_eos"] = b; });
           }},
          {"created",
           [this](Json::Value &data, const std::string& k, const std::string& v) {
             UpdateNumericField(k, v, [&data](float f) {
               data["created"] = static_cast<uint64_t>(f);
             });
           }},
      };

  if (auto it = updaters.find(key); it != updaters.end()) {
    it->second(data, key, value);
    CLI_LOG("Updated " << key << " to: " << value);
  } else {
    CLI_LOG("Warning: Unknown configuration key '" << key << "' ignored.");
  }
}

void ModelUpdCmd::UpdateVectorField(
    const std::string& key, const std::string& value,
    std::function<void(const std::vector<std::string>&)> setter) {
  std::vector<std::string> tokens;
  std::istringstream iss(value);
  std::string token;
  while (std::getline(iss, token, ',')) {
    tokens.push_back(token);
  }
  setter(tokens);
}

void ModelUpdCmd::UpdateNumericField(const std::string& key,
                                     const std::string& value,
                                     std::function<void(float)> setter) {
  try {
    float numericValue = std::stof(value);
    setter(numericValue);
  } catch (const std::exception& e) {
    CLI_LOG("Failed to parse numeric value for " << key << ": " << e.what());
  }
}

void ModelUpdCmd::UpdateBooleanField(const std::string& key,
                                     const std::string& value,
                                     std::function<void(bool)> setter) {
  bool boolValue = (value == "true" || value == "1");
  setter(boolValue);
}
}  // namespace commands
