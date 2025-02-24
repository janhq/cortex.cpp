#include "thread_fs_repository.h"
#include <algorithm>
#include <fstream>
#include <mutex>
#include "common/assistant.h"
#include "utils/result.hpp"

cpp::result<std::vector<OpenAi::Thread>, std::string>
ThreadFsRepository::ListThreads(uint8_t limit, const std::string& order,
                                const std::string& after,
                                const std::string& before) const {
  std::vector<OpenAi::Thread> threads;

  try {
    auto thread_container_path = data_folder_path_ / kThreadContainerFolderName;
    std::vector<OpenAi::Thread> all_threads;

    // First load all valid threads
    for (const auto& entry :
         std::filesystem::directory_iterator(thread_container_path)) {
      if (!entry.is_directory())
        continue;

      auto thread_file = entry.path() / kThreadFileName;
      if (!std::filesystem::exists(thread_file))
        continue;

      auto current_thread_id = entry.path().filename().string();

      // Apply pagination filters
      if (!after.empty() && current_thread_id <= after)
        continue;
      if (!before.empty() && current_thread_id >= before)
        continue;

      std::shared_lock thread_lock(GrabThreadMutex(current_thread_id));
      auto thread_result = LoadThread(current_thread_id);

      if (thread_result.has_value()) {
        all_threads.push_back(std::move(thread_result.value()));
      }

      thread_lock.unlock();
    }

    // Sort threads based on order parameter using created_at
    if (order == "desc") {
      std::sort(all_threads.begin(), all_threads.end(),
                [](const OpenAi::Thread& a, const OpenAi::Thread& b) {
                  return a.created_at > b.created_at;  // Descending order
                });
    } else {
      std::sort(all_threads.begin(), all_threads.end(),
                [](const OpenAi::Thread& a, const OpenAi::Thread& b) {
                  return a.created_at < b.created_at;  // Ascending order
                });
    }

    // Apply limit
    size_t thread_count =
        std::min(static_cast<size_t>(limit), all_threads.size());
    for (size_t i = 0; i < thread_count; i++) {
      threads.push_back(std::move(all_threads[i]));
    }

    return threads;
  } catch (const std::exception& e) {
    return cpp::fail(std::string("Failed to list threads: ") + e.what());
  }
}

std::shared_mutex& ThreadFsRepository::GrabThreadMutex(
    const std::string& thread_id) const {
  std::shared_lock map_lock(map_mutex_);
  auto it = thread_mutexes_.find(thread_id);
  if (it != thread_mutexes_.end()) {
    return *it->second;
  }

  map_lock.unlock();
  std::unique_lock map_write_lock(map_mutex_);
  return *thread_mutexes_
              .try_emplace(thread_id, std::make_unique<std::shared_mutex>())
              .first->second;
}

std::filesystem::path ThreadFsRepository::GetThreadPath(
    const std::string& thread_id) const {
  return data_folder_path_ / kThreadContainerFolderName / thread_id;
}

cpp::result<OpenAi::Thread, std::string> ThreadFsRepository::LoadThread(
    const std::string& thread_id) const {
  auto path = GetThreadPath(thread_id) / kThreadFileName;
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

    return OpenAi::Thread::FromJson(root);
  } catch (const std::exception& e) {
    return cpp::fail("Failed to load thread: " + std::string(e.what()));
  }
}

cpp::result<void, std::string> ThreadFsRepository::CreateThread(
    OpenAi::Thread& thread) {
  CTL_INF("CreateThread: " + thread.id);
  std::unique_lock lock(GrabThreadMutex(thread.id));
  auto thread_path = GetThreadPath(thread.id);

  if (std::filesystem::exists(thread_path)) {
    return cpp::fail("Thread exists: " + thread.id);
  }

  std::filesystem::create_directories(thread_path);
  auto thread_file_path = thread_path / kThreadFileName;
  std::ofstream thread_file(thread_file_path);
  thread_file.close();

  return SaveThread(thread);
}

cpp::result<void, std::string> ThreadFsRepository::SaveThread(
    OpenAi::Thread& thread) {
  auto path = GetThreadPath(thread.id) / kThreadFileName;
  if (!std::filesystem::exists(path)) {
    return cpp::fail("Path does not exist: " + path.string());
  }

  std::ofstream file(path);
  try {
    if (!file) {
      return cpp::fail("Failed to open file: " + path.string());
    }
    file << thread.ToJson()->toStyledString();
    file.flush();
    file.close();
    return {};
  } catch (const std::exception& e) {
    file.close();
    return cpp::fail("Failed to save thread: " + std::string(e.what()));
  }
}

cpp::result<OpenAi::Thread, std::string> ThreadFsRepository::RetrieveThread(
    const std::string& thread_id) const {
  std::shared_lock lock(GrabThreadMutex(thread_id));
  return LoadThread(thread_id);
}

cpp::result<void, std::string> ThreadFsRepository::ModifyThread(
    OpenAi::Thread& thread) {
  std::unique_lock lock(GrabThreadMutex(thread.id));
  auto thread_path = GetThreadPath(thread.id);

  if (!std::filesystem::exists(thread_path)) {
    return cpp::fail("Thread doesn't exist: " + thread.id);
  }

  return SaveThread(thread);
}

cpp::result<void, std::string> ThreadFsRepository::DeleteThread(
    const std::string& thread_id) {
  CTL_INF("DeleteThread: " + thread_id);

  {
    std::unique_lock thread_lock(GrabThreadMutex(thread_id));
    auto path = GetThreadPath(thread_id);
    if (!std::filesystem::exists(path)) {
      return cpp::fail("Thread doesn't exist: " + thread_id);
    }
    try {
      std::filesystem::remove_all(path);
    } catch (const std::exception& e) {
      return cpp::fail(std::string("Failed to delete thread: ") + e.what());
    }
  }

  std::unique_lock map_lock(map_mutex_);
  thread_mutexes_.erase(thread_id);
  return {};
}

cpp::result<OpenAi::JanAssistant, std::string>
ThreadFsRepository::LoadAssistant(const std::string& thread_id) const {
  auto path = GetThreadPath(thread_id) / kThreadFileName;
  if (!std::filesystem::exists(path)) {
    return cpp::fail("Path does not exist: " + path.string());
  }

  std::shared_lock thread_lock(GrabThreadMutex(thread_id));
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

    Json::Value assistants = root["assistants"];
    if (!assistants.isArray()) {
      return cpp::fail("Assistants field is not an array");
    }

    if (assistants.empty()) {
      return cpp::fail("Assistant not found in thread: " + thread_id);
    }

    return OpenAi::JanAssistant::FromJson(std::move(assistants[0]));
  } catch (const std::exception& e) {
    return cpp::fail("Failed to load assistant: " + std::string(e.what()));
  }
}

cpp::result<OpenAi::JanAssistant, std::string>
ThreadFsRepository::ModifyAssistant(const std::string& thread_id,
                                    const OpenAi::JanAssistant& assistant) {
  std::unique_lock lock(GrabThreadMutex(thread_id));

  // Load the existing thread
  auto thread_result = LoadThread(thread_id);
  if (!thread_result.has_value()) {
    return cpp::fail("Failed to load thread: " + thread_result.error());
  }

  auto& thread = thread_result.value();
  if (thread.ToJson()
          ->get("assistants", Json::Value(Json::arrayValue))
          .empty()) {
    return cpp::fail("No assistants found in thread: " + thread_id);
  }

  thread.assistants = {assistant};

  auto save_result = SaveThread(thread);
  if (!save_result.has_value()) {
    return cpp::fail("Failed to save thread: " + save_result.error());
  }

  return assistant;
}

cpp::result<void, std::string> ThreadFsRepository::CreateAssistant(
    const std::string& thread_id, const OpenAi::JanAssistant& assistant) {
  std::unique_lock lock(GrabThreadMutex(thread_id));

  // Load the existing thread
  auto thread_result = LoadThread(thread_id);
  if (!thread_result.has_value()) {
    return cpp::fail("Failed to load thread: " + thread_result.error());
  }

  auto& thread = thread_result.value();
  thread.assistants = {assistant};

  // Save the modified thread
  return SaveThread(thread);
}
