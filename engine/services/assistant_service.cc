#include "assistant_service.h"
#include <variant>
#include "utils/logging_utils.h"
#include "utils/ulid_generator.h"

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

cpp::result<std::vector<OpenAi::Assistant>, std::string>
AssistantService::ListAssistants(uint8_t limit, const std::string& order,
                                 const std::string& after,
                                 const std::string& before) const {
  CTL_INF("List assistants invoked");
  return assistant_repository_->ListAssistants(limit, order, after, before);
}

cpp::result<OpenAi::Assistant, std::string> AssistantService::CreateAssistantV2(
    const dto::CreateAssistantDto& create_dto) {

  OpenAi::Assistant assistant;
  assistant.id = "asst_" + ulid::GenerateUlid();
  assistant.model = create_dto.model;
  if (create_dto.name) {
    assistant.name = *create_dto.name;
  }
  if (create_dto.description) {
    assistant.description = *create_dto.description;
  }
  if (create_dto.instructions) {
    assistant.instructions = *create_dto.instructions;
  }
  if (create_dto.metadata) {
    assistant.metadata = *create_dto.metadata;
  }
  if (create_dto.temperature) {
    assistant.temperature = *create_dto.temperature;
  }
  if (create_dto.top_p) {
    assistant.top_p = *create_dto.top_p;
  }
  for (auto& tool_ptr : create_dto.tools) {
    // Create a new unique_ptr in assistant.tools that takes ownership
    if (auto* function_tool =
            dynamic_cast<OpenAi::AssistantFunctionTool*>(tool_ptr.get())) {
      assistant.tools.push_back(std::make_unique<OpenAi::AssistantFunctionTool>(
          std::move(*function_tool)));
    } else if (auto* code_tool =
                   dynamic_cast<OpenAi::AssistantCodeInterpreterTool*>(
                       tool_ptr.get())) {
      assistant.tools.push_back(
          std::make_unique<OpenAi::AssistantCodeInterpreterTool>(
              std::move(*code_tool)));
    } else if (auto* search_tool =
                   dynamic_cast<OpenAi::AssistantFileSearchTool*>(
                       tool_ptr.get())) {
      assistant.tools.push_back(
          std::make_unique<OpenAi::AssistantFileSearchTool>(
              std::move(*search_tool)));
    }
  }
  if (create_dto.tool_resources) {
    if (auto* code_interpreter = dynamic_cast<OpenAi::CodeInterpreter*>(
            create_dto.tool_resources.get())) {
      assistant.tool_resources = std::make_unique<OpenAi::CodeInterpreter>(
          std::move(*code_interpreter));
    } else if (auto* file_search = dynamic_cast<OpenAi::FileSearch*>(
                   create_dto.tool_resources.get())) {
      assistant.tool_resources =
          std::make_unique<OpenAi::FileSearch>(std::move(*file_search));
    }
  }
  if (create_dto.response_format) {
    assistant.response_format = *create_dto.response_format;
  }
  auto seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  assistant.created_at = seconds_since_epoch;
  return assistant_repository_->CreateAssistant(assistant);
}
cpp::result<OpenAi::Assistant, std::string>
AssistantService::RetrieveAssistantV2(const std::string& assistant_id) const {
  if (assistant_id.empty()) {
    return cpp::fail("Assistant ID cannot be empty");
  }

  return assistant_repository_->RetrieveAssistant(assistant_id);
}

cpp::result<OpenAi::Assistant, std::string> AssistantService::ModifyAssistantV2(
    const std::string& assistant_id,
    const dto::UpdateAssistantDto& update_dto) {
  if (assistant_id.empty()) {
    return cpp::fail("Assistant ID cannot be empty");
  }

  if (!update_dto.Validate()) {
    return cpp::fail("Invalid update assistant dto");
  }

  // First retrieve the existing assistant
  auto existing_assistant =
      assistant_repository_->RetrieveAssistant(assistant_id);
  if (existing_assistant.has_error()) {
    return cpp::fail(existing_assistant.error());
  }

  OpenAi::Assistant updated_assistant;
  updated_assistant.id = assistant_id;

  // Update fields if they are present in the DTO
  if (update_dto.model) {
    updated_assistant.model = *update_dto.model;
  }
  if (update_dto.name) {
    updated_assistant.name = *update_dto.name;
  }
  if (update_dto.description) {
    updated_assistant.description = *update_dto.description;
  }
  if (update_dto.instructions) {
    updated_assistant.instructions = *update_dto.instructions;
  }
  if (update_dto.metadata) {
    updated_assistant.metadata = *update_dto.metadata;
  }
  if (update_dto.temperature) {
    updated_assistant.temperature = *update_dto.temperature;
  }
  if (update_dto.top_p) {
    updated_assistant.top_p = *update_dto.top_p;
  }
  for (auto& tool_ptr : update_dto.tools) {
    if (auto* function_tool =
            dynamic_cast<OpenAi::AssistantFunctionTool*>(tool_ptr.get())) {
      updated_assistant.tools.push_back(
          std::make_unique<OpenAi::AssistantFunctionTool>(
              std::move(*function_tool)));
    } else if (auto* code_tool =
                   dynamic_cast<OpenAi::AssistantCodeInterpreterTool*>(
                       tool_ptr.get())) {
      updated_assistant.tools.push_back(
          std::make_unique<OpenAi::AssistantCodeInterpreterTool>(
              std::move(*code_tool)));
    } else if (auto* search_tool =
                   dynamic_cast<OpenAi::AssistantFileSearchTool*>(
                       tool_ptr.get())) {
      updated_assistant.tools.push_back(
          std::make_unique<OpenAi::AssistantFileSearchTool>(
              std::move(*search_tool)));
    }
  }
  if (update_dto.tool_resources) {
    if (auto* code_interpreter = dynamic_cast<OpenAi::CodeInterpreter*>(
            update_dto.tool_resources.get())) {
      updated_assistant.tool_resources =
          std::make_unique<OpenAi::CodeInterpreter>(
              std::move(*code_interpreter));
    } else if (auto* file_search = dynamic_cast<OpenAi::FileSearch*>(
                   update_dto.tool_resources.get())) {
      updated_assistant.tool_resources =
          std::make_unique<OpenAi::FileSearch>(std::move(*file_search));
    }
  }
  if (update_dto.response_format) {
    updated_assistant.response_format = *update_dto.response_format;
  }

  auto res = assistant_repository_->ModifyAssistant(updated_assistant);
  if (res.has_error()) {
    return cpp::fail(res.error());
  }

  return updated_assistant;
}

cpp::result<void, std::string> AssistantService::DeleteAssistantV2(
    const std::string& assistant_id) {
  if (assistant_id.empty()) {
    return cpp::fail("Assistant ID cannot be empty");
  }

  return assistant_repository_->DeleteAssistant(assistant_id);
}
