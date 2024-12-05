#pragma once

#include <filesystem>
#include <shared_mutex>
#include <unordered_map>
#include "common/repository/thread_repository.h"
#include "common/thread.h"
#include "utils/logging_utils.h"

class ThreadFsRepository : public ThreadRepository {
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

  cpp::result<void, std::string> SaveThread(OpenAi::Thread& thread);

 public:
  explicit ThreadFsRepository(const std::filesystem::path& data_folder_path)
      : data_folder_path_{data_folder_path} {
    CTL_INF("Constructing ThreadFsRepository..");
    auto thread_container_path = data_folder_path_ / kThreadContainerFolderName;

    if (!std::filesystem::exists(thread_container_path)) {
      std::filesystem::create_directories(thread_container_path);
    }
  }

  cpp::result<void, std::string> CreateThread(OpenAi::Thread& thread) override;

  cpp::result<std::vector<OpenAi::Thread>, std::string> ListThreads(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const override;

  cpp::result<OpenAi::Thread, std::string> RetrieveThread(
      const std::string& thread_id) const override;

  cpp::result<void, std::string> ModifyThread(OpenAi::Thread& thread) override;

  cpp::result<void, std::string> DeleteThread(
      const std::string& thread_id) override;

  ~ThreadFsRepository() = default;
};
