#include "message_fs_repository.h"
#include <algorithm>
#include <fstream>
#include <mutex>
#include "utils/result.hpp"

std::filesystem::path MessageFsRepository::GetMessagePath(
    const std::string& thread_id) const {
  return data_folder_path_ / kThreadContainerFolderName / thread_id /
         kMessageFile;
}

cpp::result<void, std::string> MessageFsRepository::CreateMessage(
    OpenAi::Message& message) {
  CTL_INF("CreateMessage for thread " + message.thread_id);
  auto path = GetMessagePath(message.thread_id);

  std::ofstream file(path, std::ios::app);
  if (!file) {
    return cpp::fail("Failed to open file for writing: " + path.string());
  }

  auto mutex = GrabMutex(message.thread_id);
  std::shared_lock<std::shared_mutex> lock(*mutex);

  auto json_str = message.ToSingleLineJsonString();
  if (json_str.has_error()) {
    return cpp::fail(json_str.error());
  }
  file << json_str.value();

  file.flush();
  if (file.fail()) {
    return cpp::fail("Failed to write to file: " + path.string());
  }
  file.close();
  if (file.fail()) {
    return cpp::fail("Failed to close file after writing: " + path.string());
  }

  return {};
}

cpp::result<std::vector<OpenAi::Message>, std::string>
MessageFsRepository::ListMessages(const std::string& thread_id, uint8_t limit,
                                  const std::string& order,
                                  const std::string& after,
                                  const std::string& before,
                                  const std::string& run_id) const {
  CTL_INF("Listing messages for thread " + thread_id);
  auto path = GetMessagePath(thread_id);

  auto mutex = GrabMutex(thread_id);
  std::shared_lock<std::shared_mutex> lock(*mutex);

  auto read_result = ReadMessageFromFile(thread_id);
  if (read_result.has_error()) {
    return cpp::fail(read_result.error());
  }

  std::vector<OpenAi::Message> messages = std::move(read_result.value());

  if (!run_id.empty()) {
    messages.erase(std::remove_if(messages.begin(), messages.end(),
                                  [&run_id](const OpenAi::Message& msg) {
                                    return msg.run_id != run_id;
                                  }),
                   messages.end());
  }

  std::sort(messages.begin(), messages.end(),
            [&order](const OpenAi::Message& a, const OpenAi::Message& b) {
              if (order == "desc") {
                return a.created_at > b.created_at;
              }
              return a.created_at < b.created_at;
            });

  auto start_it = messages.begin();
  auto end_it = messages.end();

  if (!after.empty()) {
    start_it = std::find_if(
        messages.begin(), messages.end(),
        [&after](const OpenAi::Message& msg) { return msg.id == after; });
    if (start_it != messages.end()) {
      ++start_it;  // Start from the message after the 'after' message
    } else {
      start_it = messages.begin();
    }
  }

  if (!before.empty()) {
    end_it = std::find_if(
        messages.begin(), messages.end(),
        [&before](const OpenAi::Message& msg) { return msg.id == before; });
  }

  std::vector<OpenAi::Message> result;
  size_t distance = std::distance(start_it, end_it);
  size_t limit_size = static_cast<size_t>(limit);
  CTL_INF("Distance: " + std::to_string(distance) +
          ", limit_size: " + std::to_string(limit_size));
  result.reserve(distance < limit_size ? distance : limit_size);

  for (auto it = start_it; it != end_it && result.size() < limit_size; ++it) {
    result.push_back(std::move(*it));
  }

  return result;
}

cpp::result<OpenAi::Message, std::string> MessageFsRepository::RetrieveMessage(
    const std::string& thread_id, const std::string& message_id) const {
  auto path = GetMessagePath(thread_id);

  auto mutex = GrabMutex(thread_id);
  std::unique_lock<std::shared_mutex> lock(*mutex);

  auto messages = ReadMessageFromFile(thread_id);
  if (messages.has_error()) {
    return cpp::fail(messages.error());
  }

  for (auto& msg : messages.value()) {
    if (msg.id == message_id) {
      return std::move(msg);
    }
  }

  return cpp::fail("Message not found");
}

cpp::result<void, std::string> MessageFsRepository::ModifyMessage(
    OpenAi::Message& message) {
  auto path = GetMessagePath(message.thread_id);

  auto mutex = GrabMutex(message.thread_id);
  std::unique_lock<std::shared_mutex> lock(*mutex);

  auto messages = ReadMessageFromFile(message.thread_id);
  if (messages.has_error()) {
    return cpp::fail(messages.error());
  }

  std::ofstream file(path, std::ios::trunc);
  if (!file) {
    return cpp::fail("Failed to open file for writing: " + path.string());
  }

  bool found = false;
  for (auto& msg : messages.value()) {
    if (msg.id == message.id) {
      file << message.ToSingleLineJsonString().value();
      found = true;
    } else {
      file << msg.ToSingleLineJsonString().value();
    }
  }

  file.flush();
  if (file.fail()) {
    return cpp::fail("Failed to write to file: " + path.string());
  }
  file.close();
  if (file.fail()) {
    return cpp::fail("Failed to close file after writing: " + path.string());
  }

  if (!found) {
    return cpp::fail("Message not found");
  }
  return {};
}

cpp::result<void, std::string> MessageFsRepository::DeleteMessage(
    const std::string& thread_id, const std::string& message_id) {
  auto path = GetMessagePath(thread_id);

  auto mutex = GrabMutex(thread_id);
  std::unique_lock<std::shared_mutex> lock(*mutex);
  auto messages = ReadMessageFromFile(thread_id);
  if (messages.has_error()) {
    return cpp::fail(messages.error());
  }

  std::ofstream file(path, std::ios::trunc);
  if (!file) {
    return cpp::fail("Failed to open file for writing: " + path.string());
  }

  bool found = false;
  for (auto& msg : messages.value()) {
    if (msg.id != message_id) {
      file << msg.ToSingleLineJsonString().value();
    } else {
      found = true;
    }
  }

  file.flush();
  if (file.fail()) {
    return cpp::fail("Failed to write to file: " + path.string());
  }
  file.close();
  if (file.fail()) {
    return cpp::fail("Failed to close file after writing: " + path.string());
  }

  if (!found) {
    return cpp::fail("Message not found");
  }

  return {};
}

cpp::result<std::vector<OpenAi::Message>, std::string>
MessageFsRepository::ReadMessageFromFile(const std::string& thread_id) const {
  LOG_TRACE << "Reading messages from file for thread " << thread_id;
  auto path = GetMessagePath(thread_id);

  std::ifstream file(path);
  if (!file) {
    return cpp::fail("Failed to open file: " + path.string());
  }

  std::vector<OpenAi::Message> messages;
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;
    auto msg_parse_result = OpenAi::Message::FromJsonString(std::move(line));
    if (msg_parse_result.has_error()) {
      CTL_WRN("Failed to parse message: " + msg_parse_result.error());
      continue;
    }

    messages.push_back(std::move(msg_parse_result.value()));
  }

  return messages;
}

std::shared_mutex* MessageFsRepository::GrabMutex(
    const std::string& thread_id) const {
  std::lock_guard<std::mutex> lock(mutex_map_mutex_);
  auto& thread_mutex = thread_mutexes_[thread_id];
  if (!thread_mutex) {
    thread_mutex = std::make_unique<std::shared_mutex>();
  }
  return thread_mutex.get();
}

cpp::result<void, std::string> MessageFsRepository::InitializeMessages(
    const std::string& thread_id,
    std::optional<std::vector<OpenAi::Message>> messages) {
  CTL_INF("Initializing messages for thread " + thread_id);

  auto path = GetMessagePath(thread_id);

  if (!std::filesystem::exists(path.parent_path())) {
    return cpp::fail(
        "Failed to initialize messages, thread is not created yet! Path does "
        "not exist: " +
        path.parent_path().string());
  }

  auto mutex = GrabMutex(thread_id);
  std::unique_lock<std::shared_mutex> lock(*mutex);

  std::ofstream file(path, std::ios::trunc);
  if (!file) {
    return cpp::fail("Failed to create message file: " + path.string());
  }

  if (messages.has_value()) {
    for (auto& message : messages.value()) {
      auto json_str = message.ToSingleLineJsonString();
      if (json_str.has_error()) {
        CTL_WRN("Failed to serialize message: " + json_str.error());
        continue;
      }
      file << json_str.value();
    }
  }

  file.flush();
  if (file.fail()) {
    return cpp::fail("Failed to write to file: " + path.string());
  }

  file.close();
  if (file.fail()) {
    return cpp::fail("Failed to close file after writing: " + path.string());
  }

  return {};
}
