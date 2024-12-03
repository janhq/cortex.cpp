#include "thread_service.h"
#include "utils/logging_utils.h"
#include "utils/ulid/ulid.hh"

cpp::result<OpenAi::Thread, std::string> ThreadService::CreateThread(
    std::optional<std::unique_ptr<OpenAi::ThreadToolResources>> tool_resources,
    std::optional<Cortex::VariantMap> metadata) {
  LOG_TRACE << "CreateThread";

  auto seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();

  auto ulid = ulid::CreateNowRand();
  auto thread_id = ulid::Marshal(ulid);

  OpenAi::Thread thread;
  thread.id = thread_id;
  thread.object = "thread";
  thread.created_at = seconds_since_epoch;

  if (tool_resources.has_value()) {
    thread.tool_resources = std::move(tool_resources.value());
  }
  thread.metadata = metadata.value_or(Cortex::VariantMap{});

  if (auto res = thread_repository_->CreateThread(thread); res.has_error()) {
    return cpp::fail("Failed to create message: " + res.error());
  }

  return thread;
}

cpp::result<std::vector<OpenAi::Thread>, std::string>
ThreadService::ListThreads(uint8_t limit, const std::string& order,
                           const std::string& after,
                           const std::string& before) const {
  CTL_INF("ListThreads");
  return thread_repository_->ListThreads(limit, order, after, before);
}

cpp::result<OpenAi::Thread, std::string> ThreadService::RetrieveThread(
    const std::string& thread_id) const {
  LOG_TRACE << "RetriveThread: " << thread_id;
  return thread_repository_->RetrieveThread(thread_id);
}

cpp::result<OpenAi::Thread, std::string> ThreadService::ModifyThread(
    const std::string& thread_id,
    std::optional<std::unique_ptr<OpenAi::ThreadToolResources>> tool_resources,
    std::optional<Cortex::VariantMap> metadata) {
  LOG_TRACE << "ModifyThread " << thread_id;
  auto retrieve_res = RetrieveThread(thread_id);
  if (retrieve_res.has_error()) {
    return cpp::fail("Failed to retrieve thread: " + retrieve_res.error());
  }

  retrieve_res->tool_resources = std::move(tool_resources.value());
  retrieve_res->metadata = std::move(metadata.value());

  auto res = thread_repository_->ModifyThread(retrieve_res.value());
  if (res.has_error()) {
    CTL_ERR("Failed to modify thread: " + res.error());
    return cpp::fail("Failed to modify thread: " + res.error());
  } else {
    return RetrieveThread(thread_id);
  }
}

cpp::result<std::string, std::string> ThreadService::DeleteThread(
    const std::string& thread_id) {
  LOG_TRACE << "DeleteThread: " + thread_id;
  auto res = thread_repository_->DeleteThread(thread_id);
  if (res.has_error()) {
    LOG_ERROR << "Failed to delete thread: " + res.error();
    return cpp::fail("Failed to delete thread: " + res.error());
  } else {
    return thread_id;
  }
}
