#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
using namespace std;

#include "yaml-cpp/yaml.h"
#include "yaml_config.h"

namespace config {
// Method to read YAML file
void YamlHandler::Reset() {
  model_config_ = ModelConfig();
  yaml_node_.reset();
};
void YamlHandler::ReadYamlFile(const std::string& file_path) {
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
        v.emplace_back(s.substr(0, s.find_last_of('/')) + "/model.gguf");
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
  ModelConfigFromYaml();
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
    outFile << yaml_node_;
    outFile.close();
  } catch (const std::exception& e) {
    std::cerr << "Error writing to file: " << e.what() << std::endl;
    throw;
  }
}
}  // namespace config