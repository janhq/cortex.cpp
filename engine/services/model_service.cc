#include "model_service.h"
#include <filesystem>
#include <iostream>
#include <ostream>
#include "commands/cmd_info.h"
#include "utils/cortexso_parser.h"
#include "utils/file_manager_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/model_callback_utils.h"

void PrintMenu(const std::vector<std::string>& options) {
  auto index{1};
  for (const auto& option : options) {
    std::cout << index << ". " << option << "\n";
    index++;
  }
  std::endl(std::cout);
}

std::optional<std::string> PrintSelection(
    const std::vector<std::string>& options) {
  std::string selection{""};
  PrintMenu(options);
  std::cin >> selection;

  if (selection.empty()) {
    return std::nullopt;
  }

  // std::cout << "Selection: " << selection << "\n";
  // std::cout << "Int representaion: " << std::stoi(selection) << "\n";
  if (std::stoi(selection) > options.size() || std::stoi(selection) < 1) {
    return std::nullopt;
  }

  return options[std::stoi(selection) - 1];
}

void ModelService::DownloadModel(const std::string& input) {
  if (input.empty()) {
    throw std::runtime_error(
        "Input must be Cortex Model Hub handle or HuggingFace url!");
  }

  if (input.starts_with("https://")) {
    return DownloadModelByDirectUrl(input);
  }

  // if input contains / then handle it differently
  if (input.find("/") != std::string::npos) {
    // TODO: what if we have more than one /?
    // TODO: what if the left size of / is cortexso?

    // split by /. TODO: Move this function to somewhere else
    std::string model_input = input;
    std::string delimiter{"/"};
    std::string token{""};
    std::vector<std::string> parsed{};
    std::string author{""};
    std::string model_name{""};
    while (token != model_input) {
      token = model_input.substr(0, model_input.find_first_of("/"));
      model_input = model_input.substr(model_input.find_first_of("/") + 1);
      std::string new_str{token};
      parsed.push_back(new_str);
    }

    author = parsed[0];
    model_name = parsed[1];
    auto repo_info =
        huggingface_utils::GetHuggingFaceModelRepoInfo(author, model_name);
    if (!repo_info.has_value()) {
      // throw is better?
      CTL_ERR("Model not found");
      return;
    }

    if (!repo_info->gguf.has_value()) {
      throw std::runtime_error(
          "Not a GGUF model. Currently, only GGUF single file is supported.");
    }

    std::vector<std::string> options{};
    for (const auto& sibling : repo_info->siblings) {
      if (sibling.rfilename.ends_with(".gguf")) {
        options.push_back(sibling.rfilename);
      }
    }
    auto selection = PrintSelection(options);
    std::cout << "Selected: " << selection.value() << std::endl;

    auto download_url = huggingface_utils::GetDownloadableUrl(
        author, model_name, selection.value());

    std::cout << "Download url: " << download_url << std::endl;
    // TODO: split to this function
    // DownloadHuggingFaceGgufModel(author, model_name, nullptr);
    return;
  }

  // user just input a text, seems like a model name only, maybe comes with a branch, using : as delimeter
  // handle cortexso here
  // separate into another function and the above can route to it if we regconize a cortexso url
}

std::optional<config::ModelConfig> ModelService::GetDownloadedModel(
    const std::string& modelId) const {
  auto models_path = file_manager_utils::GetModelsContainerPath();
  if (!std::filesystem::exists(models_path) ||
      !std::filesystem::is_directory(models_path)) {
    return std::nullopt;
  }

  for (const auto& entry : std::filesystem::directory_iterator(models_path)) {
    if (entry.is_regular_file() &&
        entry.path().filename().string() == modelId &&
        entry.path().extension() == ".yaml") {
      try {
        config::YamlHandler handler;
        handler.ModelConfigFromFile(entry.path().string());
        auto model_conf = handler.GetModelConfig();
        return model_conf;
      } catch (const std::exception& e) {
        LOG_ERROR << "Error reading yaml file '" << entry.path().string()
                  << "': " << e.what();
      }
    }
  }
  return std::nullopt;
}

void ModelService::DownloadModelByDirectUrl(const std::string& url) {
  // check for malformed url
  // question: What if the url is from cortexso itself
  // answer: then route to download from cortexso
  auto url_obj = url_parser::FromUrlString(url);

  if (url_obj.host == kHuggingFaceHost) {
    // goto hugging face parser to normalize the url
    // loop through path params, replace blob to resolve if any
    if (url_obj.pathParams[2] == "blob") {
      url_obj.pathParams[2] = "resolve";
    }
  }

  // should separate this function out
  auto model_id{url_obj.pathParams[1]};
  auto file_name{url_obj.pathParams.back()};

  auto local_path =
      file_manager_utils::GetModelsContainerPath() / model_id / model_id;

  try {
    std::filesystem::create_directories(local_path.parent_path());
  } catch (const std::filesystem::filesystem_error& e) {
    // if file exist, remove it
    std::filesystem::remove(local_path.parent_path());
    std::filesystem::create_directories(local_path.parent_path());
  }

  auto download_url = url_parser::FromUrl(url_obj);
  // this assume that the model being downloaded is a single gguf file
  auto downloadTask{DownloadTask{.id = url_obj.pathParams.back(),
                                 .type = DownloadType::Model,
                                 .items = {DownloadItem{
                                     .id = url_obj.pathParams.back(),
                                     .downloadUrl = download_url,
                                     .localPath = local_path,
                                 }}}};

  auto on_finished = [](const DownloadTask& finishedTask) {
    std::cout << "Download success" << std::endl;
    auto gguf_download_item = finishedTask.items[0];
    model_callback_utils::ParseGguf(gguf_download_item);
  };

  download_service_.AddDownloadTask(downloadTask, on_finished);
}

void ModelService::DownloadModelFromCortexso(const std::string& name,
                                             const std::string& branch) {
  auto downloadTask = cortexso_parser::getDownloadTask(name, branch);
  if (downloadTask.has_value()) {
    DownloadService().AddDownloadTask(downloadTask.value(),
                                      model_callback_utils::DownloadModelCb);
    CTL_INF("Download finished");
  } else {
    CTL_ERR("Model not found");
  }
}

void ModelService::DownloadHuggingFaceGgufModel(
    const std::string& author, const std::string& modelName,
    std::optional<std::string> fileName) {
  std::cout << author << std::endl;
  std::cout << modelName << std::endl;
  // if we don't have file name, we must display a list for user to pick
  // auto repo_info =
  //     huggingface_utils::GetHuggingFaceModelRepoInfo(author, modelName);
  //
  // if (!repo_info.has_value()) {
  //   // throw is better?
  //   CTL_ERR("Model not found");
  //   return;
  // }
  //
  // for (const auto& sibling : repo_info->siblings) {
  //   std::cout << sibling.rfilename << "\n";
  // }
}
