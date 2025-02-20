#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"
#include "utils/format_utils.h"
#include "yaml_config.h"

namespace config {
// Method to read YAML file
void YamlHandler::Reset() {
  model_config_ = ModelConfig();
  yaml_node_.reset();
};
void YamlHandler::ReadYamlFile(const std::string& file_path) {
  namespace fs = std::filesystem;
  namespace fmu = file_manager_utils;

  try {
    yaml_node_ = YAML::LoadFile(file_path);
    // incase of model.yml file, we don't have files yet, create them
    if (!yaml_node_["files"]) {
      auto s = file_path;
      // normalize path
      std::replace(s.begin(), s.end(), '\\', '/');
      std::vector<std::string> v;
      if (yaml_node_["engine"] &&
          (yaml_node_["engine"].as<std::string>() == kLlamaRepo ||
           (yaml_node_["engine"].as<std::string>() == kLlamaEngine))) {
        auto abs_path = s.substr(0, s.find_last_of('/')) + "/model.gguf";
        auto rel_path = fmu::ToRelativeCortexDataPath(fs::path(abs_path));
        v.emplace_back(rel_path.string());

      } else {
        v.emplace_back(s.substr(0, s.find_last_of('/')));
      }

      // TODO(any) need to support mutiple gguf files
      yaml_node_["files"] = v;
    }
  } catch (const YAML::BadFile& e) {
    throw;
  }
}

void YamlHandler::SplitPromptTemplate(ModelConfig& mc) {
  if (mc.prompt_template.size() > 0) {
    auto& pt = mc.prompt_template;
    mc.system_template = pt.substr(0, pt.find_first_of('{'));
    // std::cout << "System template: " << mc.system_template << std::endl;
    mc.user_template =
        pt.substr(pt.find_first_of('}') + 1,
                  pt.find_last_of('{') - pt.find_first_of('}') - 1);
    // std::cout << "User template : " << mc.user_template << std::endl;
    mc.ai_template = pt.substr(pt.find_last_of('}') + 1);
    // std::cout << "Assistant template: " << mc.ai_template << std::endl;
  }
}
const ModelConfig& YamlHandler::GetModelConfig() const {
  return model_config_;
}

void YamlHandler::ModelConfigFromFile(const std::string& file_path) {
  ReadYamlFile(file_path);
  ModelConfigFromYaml();
}

void YamlHandler::ModelConfigFromYaml() {
  ModelConfig tmp;
  try {
    if (yaml_node_["name"])
      tmp.name = yaml_node_["name"].as<std::string>();
    if (yaml_node_["model"])
      tmp.model = yaml_node_["model"].as<std::string>();
    if (yaml_node_["version"])
      tmp.version = yaml_node_["version"].as<std::string>();
    if (yaml_node_["size"])
      tmp.size = yaml_node_["size"].as<uint64_t>();
    if (yaml_node_["engine"])
      tmp.engine = yaml_node_["engine"].as<std::string>();
    if (yaml_node_["prompt_template"]) {
      tmp.prompt_template = yaml_node_["prompt_template"].as<std::string>();
      SplitPromptTemplate(tmp);
    }

    if (yaml_node_["os"])
      tmp.os = yaml_node_["os"].as<std::string>();
    if (yaml_node_["gpu_arch"])
      tmp.gpu_arch = yaml_node_["gpu_arch"].as<std::string>();
    if (yaml_node_["quantization_method"])
      tmp.quantization_method =
          yaml_node_["quantization_method"].as<std::string>();
    if (yaml_node_["precision"])
      tmp.precision = yaml_node_["precision"].as<std::string>();
    if (yaml_node_["trtllm_version"])
      tmp.trtllm_version = yaml_node_["trtllm_version"].as<std::string>();
    if (yaml_node_["id"])
      tmp.id = yaml_node_["id"].as<std::string>();
    if (yaml_node_["object"])
      tmp.object = yaml_node_["object"].as<std::string>();
    if (yaml_node_["owned_by"])
      tmp.owned_by = yaml_node_["owned_by"].as<std::string>();
    if (yaml_node_["top_p"])
      tmp.top_p = yaml_node_["top_p"].as<float>();
    if (yaml_node_["temperature"])
      tmp.temperature = yaml_node_["temperature"].as<float>();
    if (yaml_node_["frequency_penalty"])
      tmp.frequency_penalty = yaml_node_["frequency_penalty"].as<float>();
    if (yaml_node_["presence_penalty"])
      tmp.presence_penalty = yaml_node_["presence_penalty"].as<float>();
    if (yaml_node_["max_tokens"])
      tmp.max_tokens = yaml_node_["max_tokens"].as<int>();
    if (yaml_node_["ngl"])
      tmp.ngl = yaml_node_["ngl"].as<int>();
    if (yaml_node_["ctx_len"])
      tmp.ctx_len = yaml_node_["ctx_len"].as<int>();
    if (yaml_node_["n_parallel"])
      tmp.n_parallel = yaml_node_["n_parallel"].as<int>();
    if (yaml_node_["cpu_threads"])
      tmp.cpu_threads = yaml_node_["cpu_threads"].as<int>();
    if (yaml_node_["tp"])
      tmp.tp = yaml_node_["tp"].as<int>();
    if (yaml_node_["stream"])
      tmp.stream = yaml_node_["stream"].as<bool>();
    if (yaml_node_["text_model"])
      tmp.tp = yaml_node_["text_model"].as<bool>();
    if (yaml_node_["stop"])
      tmp.stop = yaml_node_["stop"].as<std::vector<std::string>>();
    if (yaml_node_["files"])
      tmp.files = yaml_node_["files"].as<std::vector<std::string>>();
    if (yaml_node_["created"])
      tmp.created = yaml_node_["created"].as<std::size_t>();

    if (yaml_node_["seed"])
      tmp.seed = yaml_node_["seed"].as<int>();
    if (yaml_node_["dynatemp_range"])
      tmp.dynatemp_range = yaml_node_["dynatemp_range"].as<float>();
    if (yaml_node_["dynatemp_exponent"])
      tmp.dynatemp_exponent = yaml_node_["dynatemp_exponent"].as<float>();
    if (yaml_node_["top_k"])
      tmp.top_k = yaml_node_["top_k"].as<int>();
    if (yaml_node_["min_p"])
      tmp.min_p = yaml_node_["min_p"].as<float>();
    if (yaml_node_["tfs_z"])
      tmp.tfs_z = yaml_node_["tfs_z"].as<float>();
    if (yaml_node_["typ_p"])
      tmp.typ_p = yaml_node_["typ_p"].as<float>();
    if (yaml_node_["repeat_last_n"])
      tmp.repeat_last_n = yaml_node_["repeat_last_n"].as<int>();
    if (yaml_node_["repeat_penalty"])
      tmp.repeat_penalty = yaml_node_["repeat_penalty"].as<float>();
    if (yaml_node_["mirostat"])
      tmp.mirostat = yaml_node_["mirostat"].as<bool>();
    if (yaml_node_["mirostat_tau"])
      tmp.mirostat_tau = yaml_node_["mirostat_tau"].as<float>();
    if (yaml_node_["mirostat_eta"])
      tmp.mirostat_eta = yaml_node_["mirostat_eta"].as<float>();
    if (yaml_node_["penalize_nl"])
      tmp.penalize_nl = yaml_node_["penalize_nl"].as<bool>();
    if (yaml_node_["ignore_eos"])
      tmp.ignore_eos = yaml_node_["ignore_eos"].as<bool>();
    if (yaml_node_["n_probs"])
      tmp.n_probs = yaml_node_["n_probs"].as<int>();
    if (yaml_node_["min_keep"])
      tmp.min_keep = yaml_node_["min_keep"].as<int>();
    if (yaml_node_["grammar"])
      tmp.grammar = yaml_node_["grammar"].as<std::string>();
  } catch (const std::exception& e) {
    std::cerr << "Error when load model config : " << e.what() << std::endl;
    std::cerr << "Revert ..." << std::endl;
    return;
  }
  model_config_ = std::move(tmp);
}

void YamlHandler::UpdateModelConfig(ModelConfig new_model_config) {
  ModelConfig tmp = std::move(model_config_);
  try {
    model_config_ = std::move(new_model_config);
    yaml_node_.reset();
    if (!model_config_.name.empty())
      yaml_node_["name"] = model_config_.name;
    if (!model_config_.model.empty())
      yaml_node_["model"] = model_config_.model;
    if (!model_config_.version.empty())
      yaml_node_["version"] = model_config_.version;
    if (!model_config_.engine.empty())
      yaml_node_["engine"] = model_config_.engine;
    if (!model_config_.prompt_template.empty()) {
      yaml_node_["prompt_template"] = model_config_.prompt_template;
      SplitPromptTemplate(model_config_);
    }

    if (!model_config_.os.empty())
      yaml_node_["os"] = model_config_.os;
    if (!model_config_.gpu_arch.empty())
      yaml_node_["gpu_arch"] = model_config_.gpu_arch;
    if (!model_config_.quantization_method.empty())
      yaml_node_["quantization_method"] = model_config_.quantization_method;
    if (!model_config_.precision.empty())
      yaml_node_["precision"] = model_config_.precision;
    if (!model_config_.trtllm_version.empty())
      yaml_node_["trtllm_version"] = model_config_.trtllm_version;
    if (!model_config_.id.empty())
      yaml_node_["id"] = model_config_.id;
    if (!model_config_.object.empty())
      yaml_node_["object"] = model_config_.object;
    if (!model_config_.owned_by.empty())
      yaml_node_["owned_by"] = model_config_.owned_by;
    if (!std::isnan(model_config_.top_p))
      yaml_node_["top_p"] = model_config_.top_p;
    if (!std::isnan(model_config_.temperature))
      yaml_node_["temperature"] = model_config_.temperature;
    if (!std::isnan(model_config_.frequency_penalty))
      yaml_node_["frequency_penalty"] = model_config_.frequency_penalty;
    if (!std::isnan(model_config_.presence_penalty))
      yaml_node_["presence_penalty"] = model_config_.presence_penalty;
    if (!std::isnan(static_cast<double>(model_config_.max_tokens)))
      yaml_node_["max_tokens"] = model_config_.max_tokens;
    if (!std::isnan(static_cast<double>(model_config_.ngl)))
      yaml_node_["ngl"] = model_config_.ngl;
    if (!std::isnan(static_cast<double>(model_config_.ctx_len)))
      yaml_node_["ctx_len"] = model_config_.ctx_len;
    if (!std::isnan(static_cast<double>(model_config_.n_parallel)))
      yaml_node_["n_parallel"] = model_config_.n_parallel;
    if (!std::isnan(static_cast<double>(model_config_.cpu_threads)))
      yaml_node_["cpu_threads"] = model_config_.cpu_threads;
    if (!std::isnan(static_cast<double>(model_config_.tp)))
      yaml_node_["tp"] = model_config_.tp;
    if (!std::isnan(static_cast<double>(model_config_.stream)))
      yaml_node_["stream"] = model_config_.stream;
    if (!std::isnan(static_cast<double>(model_config_.text_model)))
      yaml_node_["text_model"] = model_config_.text_model;
    if (model_config_.stop.size() > 0)
      yaml_node_["stop"] = model_config_.stop;
    if (model_config_.files.size() > 0)
      yaml_node_["files"] = model_config_.files;

    if (!std::isnan(static_cast<double>(model_config_.seed)))
      yaml_node_["seed"] = model_config_.seed;
    if (!std::isnan(model_config_.dynatemp_range))
      yaml_node_["dynatemp_range"] = model_config_.dynatemp_range;
    if (!std::isnan(model_config_.dynatemp_exponent))
      yaml_node_["dynatemp_exponent"] = model_config_.dynatemp_exponent;
    if (!std::isnan(static_cast<double>(model_config_.top_k)))
      yaml_node_["top_k"] = model_config_.top_k;
    if (!std::isnan(model_config_.min_p))
      yaml_node_["min_p"] = model_config_.min_p;
    if (!std::isnan(model_config_.tfs_z))
      yaml_node_["tfs_z"] = model_config_.tfs_z;
    if (!std::isnan(model_config_.typ_p))
      yaml_node_["typ_p"] = model_config_.typ_p;
    if (!std::isnan(static_cast<double>(model_config_.repeat_last_n)))
      yaml_node_["repeat_last_n"] = model_config_.repeat_last_n;
    if (!std::isnan(model_config_.repeat_penalty))
      yaml_node_["repeat_penalty"] = model_config_.repeat_penalty;
    if (!std::isnan(static_cast<double>(model_config_.mirostat)))
      yaml_node_["mirostat"] = model_config_.mirostat;
    if (!std::isnan(model_config_.mirostat_tau))
      yaml_node_["mirostat_tau"] = model_config_.mirostat_tau;
    if (!std::isnan(model_config_.mirostat_eta))
      yaml_node_["mirostat_eta"] = model_config_.mirostat_eta;
    if (!std::isnan(static_cast<double>(model_config_.penalize_nl)))
      yaml_node_["penalize_nl"] = model_config_.penalize_nl;
    if (!std::isnan(static_cast<double>(model_config_.ignore_eos)))
      yaml_node_["ignore_eos"] = model_config_.ignore_eos;
    if (!std::isnan(static_cast<double>(model_config_.n_probs)))
      yaml_node_["n_probs"] = model_config_.n_probs;
    if (!std::isnan(static_cast<double>(model_config_.min_keep)))
      yaml_node_["min_keep"] = model_config_.min_keep;
    if (!model_config_.grammar.empty())
      yaml_node_["grammar"] = model_config_.grammar;

    yaml_node_["size"] = model_config_.size;

    yaml_node_["created"] = std::time(nullptr);
  } catch (const std::exception& e) {
    std::cerr << "Error when update model config : " << e.what() << std::endl;
    std::cerr << "Revert ..." << std::endl;
    model_config_ = std::move(tmp);
  }
}

// Method to write all attributes to a YAML file
void YamlHandler::WriteYamlFile(const std::string& file_path) const {
  try {
    std::ofstream out_file(file_path);
    if (!out_file) {
      throw std::runtime_error("Failed to open output file.");
    }
    // Write GENERAL GGUF METADATA
    out_file << "# BEGIN GENERAL GGUF METADATA\n";
    out_file << format_utils::WriteKeyValue(
        "id", yaml_node_["id"],
        "Model ID unique between models (author / quantization)");
    out_file << format_utils::WriteKeyValue(
        "model", yaml_node_["model"],
        "Model ID which is used for request construct - should be "
        "unique between models (author / quantization)");
    out_file << format_utils::WriteKeyValue("name", yaml_node_["name"],
                                           "metadata.general.name");
    if (yaml_node_["version"]) {
      out_file << "version: " << yaml_node_["version"].as<std::string>() << "\n";
    }
    if (yaml_node_["files"] && yaml_node_["files"].size()) {
      out_file << "files:             # Can be relative OR absolute local file "
                 "path\n";
      for (const auto& source : yaml_node_["files"]) {
        out_file << "  - " << source << "\n";
      }
    }

    out_file << "# END GENERAL GGUF METADATA\n";
    out_file << "\n";
    // Write INFERENCE PARAMETERS
    out_file << "# BEGIN INFERENCE PARAMETERS\n";
    out_file << "# BEGIN REQUIRED\n";
    if (yaml_node_["stop"] && yaml_node_["stop"].size()) {
      out_file << "stop:                # tokenizer.ggml.eos_token_id\n";
      for (const auto& stop : yaml_node_["stop"]) {
        out_file << "  - " << stop << "\n";
      }
    }

    out_file << "# END REQUIRED\n";
    out_file << "\n";
    out_file << "# BEGIN OPTIONAL\n";
    out_file << format_utils::WriteKeyValue("size", yaml_node_["size"]);
    out_file << format_utils::WriteKeyValue("stream", yaml_node_["stream"],
                                           "Default true?");
    out_file << format_utils::WriteKeyValue("top_p", yaml_node_["top_p"],
                                           "Ranges: 0 to 1");
    out_file << format_utils::WriteKeyValue(
        "temperature", yaml_node_["temperature"], "Ranges: 0 to 1");
    out_file << format_utils::WriteKeyValue(
        "frequency_penalty", yaml_node_["frequency_penalty"], "Ranges: 0 to 1");
    out_file << format_utils::WriteKeyValue(
        "presence_penalty", yaml_node_["presence_penalty"], "Ranges: 0 to 1");
    out_file << format_utils::WriteKeyValue(
        "max_tokens", yaml_node_["max_tokens"],
        "Should be default to context length");
    out_file << format_utils::WriteKeyValue("seed", yaml_node_["seed"]);
    out_file << format_utils::WriteKeyValue("dynatemp_range",
                                           yaml_node_["dynatemp_range"]);
    out_file << format_utils::WriteKeyValue("dynatemp_exponent",
                                           yaml_node_["dynatemp_exponent"]);
    out_file << format_utils::WriteKeyValue("top_k", yaml_node_["top_k"]);
    out_file << format_utils::WriteKeyValue("min_p", yaml_node_["min_p"]);
    out_file << format_utils::WriteKeyValue("tfs_z", yaml_node_["tfs_z"]);
    out_file << format_utils::WriteKeyValue("typ_p", yaml_node_["typ_p"]);
    out_file << format_utils::WriteKeyValue("repeat_last_n",
                                           yaml_node_["repeat_last_n"]);
    out_file << format_utils::WriteKeyValue("repeat_penalty",
                                           yaml_node_["repeat_penalty"]);
    out_file << format_utils::WriteKeyValue("mirostat", yaml_node_["mirostat"]);
    out_file << format_utils::WriteKeyValue("mirostat_tau",
                                           yaml_node_["mirostat_tau"]);
    out_file << format_utils::WriteKeyValue("mirostat_eta",
                                           yaml_node_["mirostat_eta"]);
    out_file << format_utils::WriteKeyValue("penalize_nl",
                                           yaml_node_["penalize_nl"]);
    out_file << format_utils::WriteKeyValue("ignore_eos",
                                           yaml_node_["ignore_eos"]);
    out_file << format_utils::WriteKeyValue("n_probs", yaml_node_["n_probs"]);
    out_file << format_utils::WriteKeyValue("min_keep", yaml_node_["min_keep"]);
    out_file << format_utils::WriteKeyValue("grammar", yaml_node_["grammar"]);
    out_file << "# END OPTIONAL\n";
    out_file << "# END INFERENCE PARAMETERS\n";
    out_file << "\n";
    // Write MODEL LOAD PARAMETERS
    out_file << "# BEGIN MODEL LOAD PARAMETERS\n";
    out_file << "# BEGIN REQUIRED\n";
    out_file << format_utils::WriteKeyValue("engine", yaml_node_["engine"],
                                           "engine to run model");
    out_file << "prompt_template:";
    out_file << " " << yaml_node_["prompt_template"] << "\n";
    out_file << "# END REQUIRED\n";
    out_file << "\n";
    out_file << "# BEGIN OPTIONAL\n";
    out_file << format_utils::WriteKeyValue(
        "ctx_len", yaml_node_["ctx_len"],
        "llama.context_length | 0 or undefined = loaded from model");
    out_file << format_utils::WriteKeyValue("n_parallel",
                                           yaml_node_["n_parallel"]);
    out_file << format_utils::WriteKeyValue("cpu_threads",
                                           yaml_node_["cpu_threads"]);
    out_file << format_utils::WriteKeyValue("ngl", yaml_node_["ngl"],
                                           "Undefined = loaded from model");
    out_file << "# END OPTIONAL\n";
    out_file << "# END MODEL LOAD PARAMETERS\n";

    out_file.close();
  } catch (const std::exception& e) {
    std::cerr << "Error writing to file: " << e.what() << std::endl;
    throw;
  }
}
}  // namespace config
