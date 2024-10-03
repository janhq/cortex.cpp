#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

#include "utils/format_utils.h"
#include "utils/file_manager_utils.h"
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
          yaml_node_["engine"].as<std::string>() == "cortex.llamacpp") {
        auto abs_path = s.substr(0, s.find_last_of('/')) + "/model.gguf";
        auto rel_path = fmu::Subtract(fs::path(abs_path), fmu::GetCortexDataPath());
        v.emplace_back(rel_path.string());
      } else {
        v.emplace_back(s.substr(0, s.find_last_of('/')));
      }

      // TODO(any) need to support mutiple gguf files
      yaml_node_["files"] = v;
    }
  } catch (const YAML::BadFile& e) {
    std::cerr << "Failed to read file: " << e.what() << std::endl;
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
    std::ofstream outFile(file_path);
    if (!outFile) {
      throw std::runtime_error("Failed to open output file.");
    }
    // Write GENERAL GGUF METADATA
    outFile << "# BEGIN GENERAL GGUF METADATA\n";
    outFile << format_utils::writeKeyValue(
        "id", yaml_node_["id"],
        "Model ID unique between models (author / quantization)");
    outFile << format_utils::writeKeyValue(
        "model", yaml_node_["model"],
        "Model ID which is used for request construct - should be "
        "unique between models (author / quantization)");
    outFile << format_utils::writeKeyValue("name", yaml_node_["name"],
                                           "metadata.general.name");
    if (yaml_node_["version"]) {
      outFile << "version: " << yaml_node_["version"].as<std::string>() << "\n";
    }
    if (yaml_node_["files"] && yaml_node_["files"].size()) {
      outFile << "files:             # can be universal protocol (models://) "
                 "OR absolute local file path (file://) OR https remote URL "
                 "(https://)\n";
      for (const auto& source : yaml_node_["files"]) {
        outFile << "  - " << source << "\n";
      }
    }

    outFile << "# END GENERAL GGUF METADATA\n";
    outFile << "\n";
    // Write INFERENCE PARAMETERS
    outFile << "# BEGIN INFERENCE PARAMETERS\n";
    outFile << "# BEGIN REQUIRED\n";
    if (yaml_node_["stop"] && yaml_node_["stop"].size()) {
      outFile << "stop:                # tokenizer.ggml.eos_token_id\n";
      for (const auto& stop : yaml_node_["stop"]) {
        outFile << "  - " << stop << "\n";
      }
    }

    outFile << "# END REQUIRED\n";
    outFile << "\n";
    outFile << "# BEGIN OPTIONAL\n";
    outFile << format_utils::writeKeyValue("stream", yaml_node_["stream"],
                                           "Default true?");
    outFile << format_utils::writeKeyValue("top_p", yaml_node_["top_p"],
                                           "Ranges: 0 to 1");
    outFile << format_utils::writeKeyValue(
        "temperature", yaml_node_["temperature"], "Ranges: 0 to 1");
    outFile << format_utils::writeKeyValue(
        "frequency_penalty", yaml_node_["frequency_penalty"], "Ranges: 0 to 1");
    outFile << format_utils::writeKeyValue(
        "presence_penalty", yaml_node_["presence_penalty"], "Ranges: 0 to 1");
    outFile << format_utils::writeKeyValue(
        "max_tokens", yaml_node_["max_tokens"],
        "Should be default to context length");
    outFile << format_utils::writeKeyValue("seed", yaml_node_["seed"]);
    outFile << format_utils::writeKeyValue("dynatemp_range",
                                           yaml_node_["dynatemp_range"]);
    outFile << format_utils::writeKeyValue("dynatemp_exponent",
                                           yaml_node_["dynatemp_exponent"]);
    outFile << format_utils::writeKeyValue("top_k", yaml_node_["top_k"]);
    outFile << format_utils::writeKeyValue("min_p", yaml_node_["min_p"]);
    outFile << format_utils::writeKeyValue("tfs_z", yaml_node_["tfs_z"]);
    outFile << format_utils::writeKeyValue("typ_p", yaml_node_["typ_p"]);
    outFile << format_utils::writeKeyValue("repeat_last_n",
                                           yaml_node_["repeat_last_n"]);
    outFile << format_utils::writeKeyValue("repeat_penalty",
                                           yaml_node_["repeat_penalty"]);
    outFile << format_utils::writeKeyValue("mirostat", yaml_node_["mirostat"]);
    outFile << format_utils::writeKeyValue("mirostat_tau",
                                           yaml_node_["mirostat_tau"]);
    outFile << format_utils::writeKeyValue("mirostat_eta",
                                           yaml_node_["mirostat_eta"]);
    outFile << format_utils::writeKeyValue("penalize_nl",
                                           yaml_node_["penalize_nl"]);
    outFile << format_utils::writeKeyValue("ignore_eos",
                                           yaml_node_["ignore_eos"]);
    outFile << format_utils::writeKeyValue("n_probs", yaml_node_["n_probs"]);
    outFile << format_utils::writeKeyValue("min_keep", yaml_node_["min_keep"]);
    outFile << format_utils::writeKeyValue("grammar", yaml_node_["grammar"]);
    outFile << "# END OPTIONAL\n";
    outFile << "# END INFERENCE PARAMETERS\n";
    outFile << "\n";
    // Write MODEL LOAD PARAMETERS
    outFile << "# BEGIN MODEL LOAD PARAMETERS\n";
    outFile << "# BEGIN REQUIRED\n";
    outFile << format_utils::writeKeyValue("engine", yaml_node_["engine"],
                                           "engine to run model");
    outFile << "prompt_template:";
    outFile << " " << yaml_node_["prompt_template"] << "\n";
    outFile << "# END REQUIRED\n";
    outFile << "\n";
    outFile << "# BEGIN OPTIONAL\n";
    outFile << format_utils::writeKeyValue(
        "ctx_len", yaml_node_["ctx_len"],
        "llama.context_length | 0 or undefined = loaded from model");
    outFile << format_utils::writeKeyValue("ngl", yaml_node_["ngl"],
                                           "Undefined = loaded from model");
    outFile << "# END OPTIONAL\n";
    outFile << "# END MODEL LOAD PARAMETERS\n";

    outFile.close();
  } catch (const std::exception& e) {
    std::cerr << "Error writing to file: " << e.what() << std::endl;
    throw;
  }
}
}  // namespace config
