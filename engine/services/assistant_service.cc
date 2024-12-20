#include "assistant_service.h"
#include "utils/logging_utils.h"

cpp::result<OpenAi::JanAssistant, std::string>
AssistantService::CreateAssistant(const std::string& thread_id,
                                  const OpenAi::JanAssistant& assistant) {
  CTL_INF("CreateAssistant: " + thread_id);
  auto res = thread_repository_->CreateAssistant(thread_id, assistant);

  if (res.has_error()) {
    return cpp::fail(res.error());
  }

  return assistant;
}

cpp::result<OpenAi::JanAssistant, std::string>
AssistantService::RetrieveAssistant(const std::string& assistant_id) const {
  CTL_INF("RetrieveAssistant: " + assistant_id);
  return thread_repository_->LoadAssistant(assistant_id);
}

cpp::result<OpenAi::JanAssistant, std::string>
AssistantService::ModifyAssistant(const std::string& thread_id,
                                  const OpenAi::JanAssistant& assistant) {
  CTL_INF("RetrieveAssistant: " + thread_id);
  return thread_repository_->ModifyAssistant(thread_id, assistant);
}
