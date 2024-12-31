#pragma once

#include <filesystem>
#include <shared_mutex>
#include <unordered_map>
#include "common/assistant.h"
#include "common/repository/thread_repository.h"
#include "common/thread.h"
#include "utils/logging_utils.h"

// this interface is for backward supporting Jan
class AssistantBackwardCompatibleSupport {
 public:
  virtual cpp::result<OpenAi::JanAssistant, std::string> LoadAssistant(
      const std::string& thread_id) const = 0;

  virtual cpp::result<OpenAi::JanAssistant, std::string> ModifyAssistant(
      const std::string& thread_id, const OpenAi::JanAssistant& assistant) = 0;

  virtual cpp::result<void, std::string> CreateAssistant(
      const std::string& thread_id, const OpenAi::JanAssistant& assistant) = 0;
};

class ThreadFsRepository : public ThreadRepository,
                           public AssistantBackwardCompatibleSupport {
 private:
  constexpr static auto kThreadFileName = "thread.json";
  constexpr static auto kThreadContainerFolderName = "threads";

  mutable std::shared_mutex map_mutex_;
  mutable std::unordered_map<std::string, std::unique_ptr<std::shared_mutex>>
      thread_mutexes_;

  /**
   * The path to the data folder.
   */
  std::filesystem::path data_folder_path_;

  std::shared_mutex& GrabThreadMutex(const std::string& thread_id) const;

  std::filesystem::path GetThreadPath(const std::string& thread_id) const;

  /**
   * Read the thread file and parse to Thread from the file system.
   */
  cpp::result<OpenAi::Thread, std::string> LoadThread(
      const std::string& thread_id) const;

  cpp::result<void, std::string> SaveThread(const OpenAi::Thread& thread);

 public:
  explicit ThreadFsRepository(const std::filesystem::path& data_folder_path)
      : data_folder_path_{data_folder_path} {
    CTL_INF("Constructing ThreadFsRepository..");
    auto thread_container_path = data_folder_path_ / kThreadContainerFolderName;

    if (!std::filesystem::exists(thread_container_path)) {
      std::filesystem::create_directories(thread_container_path);
    }
  }

  cpp::result<void, std::string> CreateThread(
      const OpenAi::Thread& thread) override;

  cpp::result<std::vector<OpenAi::Thread>, std::string> ListThreads(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const override;

  cpp::result<OpenAi::Thread, std::string> RetrieveThread(
      const std::string& thread_id) const override;

  cpp::result<void, std::string> ModifyThread(OpenAi::Thread& thread) override;

  cpp::result<void, std::string> DeleteThread(
      const std::string& thread_id) override;

  // for supporting Jan
  cpp::result<OpenAi::JanAssistant, std::string> LoadAssistant(
      const std::string& thread_id) const override;

  cpp::result<OpenAi::JanAssistant, std::string> ModifyAssistant(
      const std::string& thread_id,
      const OpenAi::JanAssistant& assistant) override;

  cpp::result<void, std::string> CreateAssistant(
      const std::string& thread_id,
      const OpenAi::JanAssistant& assistant) override;

  ~ThreadFsRepository() = default;
};
