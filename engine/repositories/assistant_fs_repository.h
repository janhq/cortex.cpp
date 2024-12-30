#pragma once

#include <filesystem>
#include <shared_mutex>

#include "common/repository/assistant_repository.h"

class AssistantFsRepository : public AssistantRepository {
 public:
  constexpr static auto kAssistantContainerFolderName = "assistants";
  constexpr static auto kAssistantFileName = "assistant.json";

  cpp::result<std::vector<OpenAi::Assistant>, std::string> ListAssistants(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const override;

  cpp::result<OpenAi::Assistant, std::string> CreateAssistant(
      OpenAi::Assistant& assistant) override;

  cpp::result<OpenAi::Assistant, std::string> RetrieveAssistant(
      const std::string assistant_id) const override;

  cpp::result<void, std::string> ModifyAssistant(
      OpenAi::Assistant& assistant) override;

  cpp::result<void, std::string> DeleteAssistant(
      const std::string& assitant_id) override;

  explicit AssistantFsRepository(const std::filesystem::path& data_folder_path)
      : data_folder_path_{data_folder_path} {
    CTL_INF("Constructing AssistantFsRepository..");
    auto path = data_folder_path_ / kAssistantContainerFolderName;

    if (!std::filesystem::exists(path)) {
      std::filesystem::create_directories(path);
    }
  }

  ~AssistantFsRepository() = default;

 private:
  std::filesystem::path GetAssistantPath(const std::string& assistant_id) const;

  std::shared_mutex& GrabAssistantMutex(const std::string& assistant_id) const;

  cpp::result<void, std::string> SaveAssistant(OpenAi::Assistant& assistant);

  cpp::result<OpenAi::Assistant, std::string> LoadAssistant(
      const std::string& assistant_id) const;

  /**
   * The path to the data folder.
   */
  std::filesystem::path data_folder_path_;

  mutable std::shared_mutex map_mutex_;
  mutable std::unordered_map<std::string, std::unique_ptr<std::shared_mutex>>
      assistant_mutexes_;
};
