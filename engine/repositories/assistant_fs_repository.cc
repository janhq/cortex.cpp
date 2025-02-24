#include "assistant_fs_repository.h"
#include <json/reader.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <mutex>
#include "utils/result.hpp"

cpp::result<std::vector<OpenAi::Assistant>, std::string>
AssistantFsRepository::ListAssistants(uint8_t limit, const std::string& order,
                                      const std::string& after,
                                      const std::string& before) const {
  std::vector<OpenAi::Assistant> assistants;
  try {
    auto assistant_container_path =
        data_folder_path_ / kAssistantContainerFolderName;
    std::vector<OpenAi::Assistant> all_assistants;

    for (const auto& entry :
         std::filesystem::directory_iterator(assistant_container_path)) {
      if (!entry.is_directory()) {
        continue;
      }

      auto assistant_file = entry.path() / kAssistantFileName;
      if (!std::filesystem::exists(assistant_file)) {
        continue;
      }

      auto current_assistant_id = entry.path().filename().string();

      if (!after.empty() && current_assistant_id <= after) {
        continue;
      }

      if (!before.empty() && current_assistant_id >= before) {
        continue;
      }

      std::shared_lock assistant_lock(GrabAssistantMutex(current_assistant_id));
      auto assistant_res = LoadAssistant(current_assistant_id);
      if (assistant_res.has_value()) {
        all_assistants.push_back(std::move(assistant_res.value()));
      }
      assistant_lock.unlock();
    }

    // sorting
    if (order == "desc") {
      std::sort(all_assistants.begin(), all_assistants.end(),
                [](const OpenAi::Assistant& assistant1,
                   const OpenAi::Assistant& assistant2) {
                  return assistant1.created_at > assistant2.created_at;
                });
    } else {
      std::sort(all_assistants.begin(), all_assistants.end(),
                [](const OpenAi::Assistant& assistant1,
                   const OpenAi::Assistant& assistant2) {
                  return assistant1.created_at < assistant2.created_at;
                });
    }

    size_t assistant_count =
        std::min(static_cast<size_t>(limit), all_assistants.size());
    for (size_t i = 0; i < assistant_count; i++) {
      assistants.push_back(std::move(all_assistants[i]));
    }

    return assistants;
  } catch (const std::exception& e) {
    return cpp::fail("Failed to list assistants: " + std::string(e.what()));
  }
}

cpp::result<OpenAi::Assistant, std::string>
AssistantFsRepository::RetrieveAssistant(const std::string assistant_id) const {
  std::shared_lock lock(GrabAssistantMutex(assistant_id));
  return LoadAssistant(assistant_id);
}

cpp::result<void, std::string> AssistantFsRepository::ModifyAssistant(
    OpenAi::Assistant& assistant) {
  {
    std::unique_lock lock(GrabAssistantMutex(assistant.id));
    auto path = GetAssistantPath(assistant.id);

    if (!std::filesystem::exists(path)) {
      lock.unlock();
      return cpp::fail("Assistant doesn't exist: " + assistant.id);
    }
  }

  return SaveAssistant(assistant);
}

cpp::result<void, std::string> AssistantFsRepository::DeleteAssistant(
    const std::string& assitant_id) {
  {
    std::unique_lock assistant_lock(GrabAssistantMutex(assitant_id));
    auto path = GetAssistantPath(assitant_id);
    if (!std::filesystem::exists(path)) {
      return cpp::fail("Assistant doesn't exist: " + assitant_id);
    }
    try {
      std::filesystem::remove_all(path);
    } catch (const std::exception& e) {
      return cpp::fail("");
    }
  }

  std::unique_lock map_lock(map_mutex_);
  assistant_mutexes_.erase(assitant_id);
  return {};
}

cpp::result<OpenAi::Assistant, std::string>
AssistantFsRepository::CreateAssistant(OpenAi::Assistant& assistant) {
  CTL_INF("CreateAssistant: " + assistant.id);
  {
    std::unique_lock lock(GrabAssistantMutex(assistant.id));
    auto path = GetAssistantPath(assistant.id);

    if (std::filesystem::exists(path)) {
      return cpp::fail("Assistant already exists: " + assistant.id);
    }

    std::filesystem::create_directories(path);
    auto assistant_file_path = path / kAssistantFileName;
    std::ofstream assistant_file(assistant_file_path);
    assistant_file.close();

    CTL_INF("CreateAssistant created new file: " + assistant.id);
    auto save_result = SaveAssistant(assistant);
    if (save_result.has_error()) {
      lock.unlock();
      return cpp::fail("Failed to save assistant: " + save_result.error());
    }
  }
  return RetrieveAssistant(assistant.id);
}

cpp::result<void, std::string> AssistantFsRepository::SaveAssistant(
    OpenAi::Assistant& assistant) {
  auto path = GetAssistantPath(assistant.id) / kAssistantFileName;
  if (!std::filesystem::exists(path)) {
    std::filesystem::create_directories(path);
  }

  std::ofstream file(path);
  if (!file) {
    return cpp::fail("Failed to open file: " + path.string());
  }
  try {
    file << assistant.ToJson()->toStyledString();
    file.flush();
    file.close();
    return {};
  } catch (const std::exception& e) {
    file.close();
    return cpp::fail("Failed to save assistant: " + std::string(e.what()));
  }
}

std::filesystem::path AssistantFsRepository::GetAssistantPath(
    const std::string& assistant_id) const {
  auto container_folder_path =
      data_folder_path_ / kAssistantContainerFolderName;
  if (!std::filesystem::exists(container_folder_path)) {
    std::filesystem::create_directories(container_folder_path);
  }

  return data_folder_path_ / kAssistantContainerFolderName / assistant_id;
}

std::shared_mutex& AssistantFsRepository::GrabAssistantMutex(
    const std::string& assistant_id) const {
  std::shared_lock map_lock(map_mutex_);
  auto it = assistant_mutexes_.find(assistant_id);
  if (it != assistant_mutexes_.end()) {
    return *it->second;
  }

  map_lock.unlock();
  std::unique_lock map_write_lock(map_mutex_);
  return *assistant_mutexes_
              .try_emplace(assistant_id, std::make_unique<std::shared_mutex>())
              .first->second;
}

cpp::result<OpenAi::Assistant, std::string>
AssistantFsRepository::LoadAssistant(const std::string& assistant_id) const {
  auto path = GetAssistantPath(assistant_id) / kAssistantFileName;
  if (!std::filesystem::exists(path)) {
    return cpp::fail("Path does not exist: " + path.string());
  }

  try {
    std::ifstream file(path);
    if (!file.is_open()) {
      return cpp::fail("Failed to open file: " + path.string());
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;

    if (!parseFromStream(builder, file, &root, &errs)) {
      return cpp::fail("Failed to parse JSON: " + errs);
    }

    return OpenAi::Assistant::FromJson(std::move(root));
  } catch (const std::exception& e) {
    return cpp::fail("Failed to load assistant: " + std::string(e.what()));
  }
}
