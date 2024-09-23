#include "model_upd_cmd.h"
#include "config/yaml_config.h"
#include "utils/logging_utils.h"
#include "utils/modellist_utils.h"
namespace commands {

ModelUpdCmd::ModelUpdCmd(std::string model_handle)
    : model_handle_(std::move(model_handle)) {}

void ModelUpdCmd::Exec(const ModelUpdateOptions& options) {
  modellist_utils::ModelListUtils model_list_utils;
  try {
    auto model_entry = model_list_utils.GetModelInfo(model_handle_);
    config::YamlHandler yaml_handler;
    yaml_handler.ModelConfigFromFile(model_entry.path_to_model_yaml);
    config::ModelConfig model_config = yaml_handler.GetModelConfig();

    // Update only the fields that were passed as arguments
    if (!options.name.empty()) {
      model_config.name = options.name;
      CLI_LOG("Updated name to: " << options.name);
    }
    if (!options.model.empty()) {
      model_config.model = options.model;
      CLI_LOG("Updated model to: " << options.model);
    }
    if (!options.version.empty()) {
      model_config.version = options.version;
      CLI_LOG("Updated version to: " << options.version);
    }
    if (!options.stop.empty()) {
      std::istringstream iss(options.stop);
      std::string token;
      model_config.stop.clear();
      while (std::getline(iss, token, ',')) {
        model_config.stop.push_back(token);
      }
      CLI_LOG("Updated stop to: " << options.stop);
    }
    if (!options.top_p.empty()) {
      model_config.top_p = std::stof(options.top_p);
      CLI_LOG("Updated top_p to: " << options.top_p);
    }
    if (!options.temperature.empty()) {
      model_config.temperature = std::stof(options.temperature);
      CLI_LOG("Updated temperature to: " << options.temperature);
    }
    if (!options.frequency_penalty.empty()) {
      model_config.frequency_penalty = std::stof(options.frequency_penalty);
      CLI_LOG("Updated frequency_penalty to: " << options.frequency_penalty);
    }
    if (!options.presence_penalty.empty()) {
      model_config.presence_penalty = std::stof(options.presence_penalty);
      CLI_LOG("Updated presence_penalty to: " << options.presence_penalty);
    }
    if (!options.max_tokens.empty()) {
      model_config.max_tokens = std::stoi(options.max_tokens);
      CLI_LOG("Updated max_tokens to: " << options.max_tokens);
    }
    if (!options.stream.empty()) {
      model_config.stream = (options.stream == "true" || options.stream == "1");
      CLI_LOG("Updated stream to: " << options.stream);
    }
    if (!options.ngl.empty()) {
      model_config.ngl = std::stoi(options.ngl);
      CLI_LOG("Updated ngl to: " << options.ngl);
    }
    if (!options.ctx_len.empty()) {
      model_config.ctx_len = std::stoi(options.ctx_len);
      CLI_LOG("Updated ctx_len to: " << options.ctx_len);
    }
    if (!options.engine.empty()) {
      model_config.engine = options.engine;
      CLI_LOG("Updated engine to: " << options.engine);
    }
    if (!options.prompt_template.empty()) {
      model_config.prompt_template = options.prompt_template;
      CLI_LOG("Updated prompt_template to: " << options.prompt_template);
    }
    if (!options.system_template.empty()) {
      model_config.system_template = options.system_template;
      CLI_LOG("Updated system_template to: " << options.system_template);
    }
    if (!options.user_template.empty()) {
      model_config.user_template = options.user_template;
      CLI_LOG("Updated user_template to: " << options.user_template);
    }
    if (!options.ai_template.empty()) {
      model_config.ai_template = options.ai_template;
      CLI_LOG("Updated ai_template to: " << options.ai_template);
    }
    if (!options.os.empty()) {
      model_config.os = options.os;
      CLI_LOG("Updated os to: " << options.os);
    }
    if (!options.gpu_arch.empty()) {
      model_config.gpu_arch = options.gpu_arch;
      CLI_LOG("Updated gpu_arch to: " << options.gpu_arch);
    }
    if (!options.quantization_method.empty()) {
      model_config.quantization_method = options.quantization_method;
      CLI_LOG(
          "Updated quantization_method to: " << options.quantization_method);
    }
    if (!options.precision.empty()) {
      model_config.precision = options.precision;
      CLI_LOG("Updated precision to: " << options.precision);
    }
    if (!options.tp.empty()) {
      model_config.tp = std::stoi(options.tp);
      CLI_LOG("Updated tp to: " << options.tp);
    }
    if (!options.trtllm_version.empty()) {
      model_config.trtllm_version = options.trtllm_version;
      CLI_LOG("Updated trtllm_version to: " << options.trtllm_version);
    }
    if (!options.text_model.empty()) {
      model_config.text_model =
          (options.text_model == "true" || options.text_model == "1");
      CLI_LOG("Updated text_model to: " << options.text_model);
    }
    if (!options.files.empty()) {
      std::istringstream iss(options.files);
      std::string token;
      model_config.files.clear();
      while (std::getline(iss, token, ',')) {
        model_config.files.push_back(token);
      }
      CLI_LOG("Updated files to: " << options.files);
    }
    if (!options.created.empty()) {
      model_config.created = std::stoull(options.created);
      CLI_LOG("Updated created to: " << options.created);
    }
    if (!options.object.empty()) {
      model_config.object = options.object;
      CLI_LOG("Updated object to: " << options.object);
    }
    if (!options.owned_by.empty()) {
      model_config.owned_by = options.owned_by;
      CLI_LOG("Updated owned_by to: " << options.owned_by);
    }
    if (!options.seed.empty()) {
      model_config.seed = std::stoi(options.seed);
      CLI_LOG("Updated seed to: " << options.seed);
    }
    if (!options.dynatemp_range.empty()) {
      model_config.dynatemp_range = std::stof(options.dynatemp_range);
      CLI_LOG("Updated dynatemp_range to: " << options.dynatemp_range);
    }
    if (!options.dynatemp_exponent.empty()) {
      model_config.dynatemp_exponent = std::stof(options.dynatemp_exponent);
      CLI_LOG("Updated dynatemp_exponent to: " << options.dynatemp_exponent);
    }
    if (!options.top_k.empty()) {
      model_config.top_k = std::stoi(options.top_k);
      CLI_LOG("Updated top_k to: " << options.top_k);
    }
    if (!options.min_p.empty()) {
      model_config.min_p = std::stof(options.min_p);
      CLI_LOG("Updated min_p to: " << options.min_p);
    }
    if (!options.tfs_z.empty()) {
      model_config.tfs_z = std::stof(options.tfs_z);
      CLI_LOG("Updated tfs_z to: " << options.tfs_z);
    }
    if (!options.typ_p.empty()) {
      model_config.typ_p = std::stof(options.typ_p);
      CLI_LOG("Updated typ_p to: " << options.typ_p);
    }
    if (!options.repeat_last_n.empty()) {
      model_config.repeat_last_n = std::stoi(options.repeat_last_n);
      CLI_LOG("Updated repeat_last_n to: " << options.repeat_last_n);
    }
    if (!options.repeat_penalty.empty()) {
      model_config.repeat_penalty = std::stof(options.repeat_penalty);
      CLI_LOG("Updated repeat_penalty to: " << options.repeat_penalty);
    }
    if (!options.mirostat.empty()) {
      model_config.mirostat =
          (options.mirostat == "true" || options.mirostat == "1");
      CLI_LOG("Updated mirostat to: " << options.mirostat);
    }
    if (!options.mirostat_tau.empty()) {
      model_config.mirostat_tau = std::stof(options.mirostat_tau);
      CLI_LOG("Updated mirostat_tau to: " << options.mirostat_tau);
    }
    if (!options.mirostat_eta.empty()) {
      model_config.mirostat_eta = std::stof(options.mirostat_eta);
      CLI_LOG("Updated mirostat_eta to: " << options.mirostat_eta);
    }
    if (!options.penalize_nl.empty()) {
      model_config.penalize_nl =
          (options.penalize_nl == "true" || options.penalize_nl == "1");
      CLI_LOG("Updated penalize_nl to: " << options.penalize_nl);
    }
    if (!options.ignore_eos.empty()) {
      model_config.ignore_eos =
          (options.ignore_eos == "true" || options.ignore_eos == "1");
      CLI_LOG("Updated ignore_eos to: " << options.ignore_eos);
    }
    if (!options.n_probs.empty()) {
      model_config.n_probs = std::stoi(options.n_probs);
      CLI_LOG("Updated n_probs to: " << options.n_probs);
    }
    if (!options.min_keep.empty()) {
      model_config.min_keep = std::stoi(options.min_keep);
      CLI_LOG("Updated min_keep to: " << options.min_keep);
    }
    if (!options.grammar.empty()) {
      model_config.grammar = options.grammar;
      CLI_LOG("Updated grammar to: " << options.grammar);
    }
    yaml_handler.UpdateModelConfig(model_config);

    yaml_handler.WriteYamlFile(model_entry.path_to_model_yaml);
    CLI_LOG("Successfully update model ID '" + model_handle_ + "'!");
  } catch (const std::exception& e) {
    CLI_LOG("Fail to update model with model ID '" + model_handle_ +
            "': " + e.what());
  }
}

}  // namespace commands