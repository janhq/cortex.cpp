#include "message_fs_repository.h"
#include "utils/file_manager_utils.h"
#include "utils/result.hpp"

namespace {
constexpr static const std::string_view kMessageFile = "messages.jsonl";

inline cpp::result<std::filesystem::path, std::string> GetMessageFileAbsPath(
    const std::string& thread_id) {
  auto path =
      file_manager_utils::GetThreadsContainerPath() / thread_id / kMessageFile;
  if (!std::filesystem::exists(path)) {
    return cpp::fail("Message file not exist at path: " + path.string());
  }
  return path;
}
}  // namespace

cpp::result<void, std::string> MessageFsRepository::CreateMessage(
    ThreadMessage::Message& message) {
  CTL_INF("CreateMessage for thread " + message.thread_id);
  auto path = GetMessageFileAbsPath(message.thread_id);
  if (path.has_error()) {
    return cpp::fail(path.error());
  }

  std::ofstream file(path->string(), std::ios::app);
  if (!file) {
    return cpp::fail("Failed to open file for writing: " + path->string());
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
    return cpp::fail("Failed to write to file: " + path->string());
  }
  file.close();
  if (file.fail()) {
    return cpp::fail("Failed to close file after writing: " + path->string());
  }

  return {};
}

cpp::result<std::vector<ThreadMessage::Message>, std::string>
MessageFsRepository::ListMessages(const std::string& thread_id, uint8_t limit,
                                  const std::string& order,
                                  const std::string& after,
                                  const std::string& before,
                                  const std::string& run_id) const {
  CTL_INF("Listing messages for thread " + thread_id);
  auto path = GetMessageFileAbsPath(thread_id);
  if (path.has_error()) {
    return cpp::fail(path.error());
  }

  auto mutex = GrabMutex(thread_id);
  std::shared_lock<std::shared_mutex> lock(*mutex);

  return ReadMessageFromFile(thread_id);
}

cpp::result<ThreadMessage::Message, std::string>
MessageFsRepository::RetrieveMessage(const std::string& thread_id,
                                     const std::string& message_id) const {
  auto path = GetMessageFileAbsPath(thread_id);
  if (path.has_error()) {
    return cpp::fail(path.error());
  }

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
    ThreadMessage::Message& message) {
  auto path = GetMessageFileAbsPath(message.thread_id);
  if (path.has_error()) {
    return cpp::fail(path.error());
  }

  auto mutex = GrabMutex(message.thread_id);
  std::unique_lock<std::shared_mutex> lock(*mutex);

  auto messages = ReadMessageFromFile(message.thread_id);
  if (messages.has_error()) {
    return cpp::fail(messages.error());
  }

  std::ofstream file(path.value().string(), std::ios::trunc);
  if (!file) {
    return cpp::fail("Failed to open file for writing: " +
                     path.value().string());
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
    return cpp::fail("Failed to write to file: " + path->string());
  }
  file.close();
  if (file.fail()) {
    return cpp::fail("Failed to close file after writing: " + path->string());
  }

  if (!found) {
    return cpp::fail("Message not found");
  }
  return {};
}

cpp::result<void, std::string> MessageFsRepository::DeleteMessage(
    const std::string& thread_id, const std::string& message_id) {
  auto path = GetMessageFileAbsPath(thread_id);
  if (path.has_error()) {
    return cpp::fail(path.error());
  }

  auto mutex = GrabMutex(thread_id);
  std::unique_lock<std::shared_mutex> lock(*mutex);
  auto messages = ReadMessageFromFile(thread_id);
  if (messages.has_error()) {
    return cpp::fail(messages.error());
  }

  std::ofstream file(path.value().string(), std::ios::trunc);
  if (!file) {
    return cpp::fail("Failed to open file for writing: " +
                     path.value().string());
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
    return cpp::fail("Failed to write to file: " + path->string());
  }
  file.close();
  if (file.fail()) {
    return cpp::fail("Failed to close file after writing: " + path->string());
  }

  if (!found) {
    return cpp::fail("Message not found");
  }

  return {};
}

cpp::result<std::vector<ThreadMessage::Message>, std::string>
MessageFsRepository::ReadMessageFromFile(const std::string& thread_id) const {
  LOG_TRACE << "Reading messages from file for thread " << thread_id;
  auto path = GetMessageFileAbsPath(thread_id);
  if (path.has_error()) {
    return cpp::fail(path.error());
  }

  std::ifstream file(path.value());
  if (!file) {
    return cpp::fail("Failed to open file: " + path->string());
  }

  std::vector<ThreadMessage::Message> messages;
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;
    auto msg_parse_result =
        ThreadMessage::Message::FromJsonString(std::move(line));
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
