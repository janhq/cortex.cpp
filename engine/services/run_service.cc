#include "run_service.h"
#include <thread>
#include "common/events/error.h"
#include "utils/ulid_generator.h"

auto RunService::CreateRun(const dto::RunCreateDto& create_dto)
    -> cpp::result<OpenAi::Run, std::string> {

  auto assistant = assistant_srv_->RetrieveAssistantV2(create_dto.assistant_id);
  if (assistant.has_error()) {
    return cpp::fail(assistant.error());
  }

  auto model = create_dto.model.has_value() ? create_dto.model.value()
                                            : assistant->model;
  // todo: check if model exists

  auto additional_inst = create_dto.additional_instructions.value_or("");
  auto instructions = create_dto.instructions.has_value()
                          ? create_dto.instructions.value()
                          : assistant->instructions.value_or("");
  instructions += ". " + additional_inst;

  // parsing messages and store it
  // tools
  // metadata

  auto temperature = create_dto.temperature.has_value()
                         ? create_dto.temperature.value()
                         : assistant->temperature.value_or(1.0f);

  auto top_p = create_dto.top_p.has_value() ? create_dto.top_p.value()
                                            : assistant->top_p.value_or(1.0f);

  auto stream =
      create_dto.stream.has_value() ? create_dto.stream.value() : true;

  // max_prompt_tokens
  // max_completion_tokens
  // truncation_strategy
  // tool_choice
  // parallel_tool_calls
  // response_format

  auto id{"run_" + ulid::GenerateUlid()};

  OpenAi::Run run;
  run.id = id;
  run.assistant_id = create_dto.assistant_id;
  run.model = model;
  run.instructions = instructions;
  run.temperature = temperature;
  run.top_p = top_p;

  // TODO: submit run
  // if submit success then we return run

  return run;
}

auto RunService::CreateRunStream(
    const dto::RunCreateDto& create_dto,
    std::function<void(const OpenAi::AssistantStreamEvent&, bool)> callback)
    -> void {
  std::thread([this, &create_dto, callback = std::move(callback)]() {
    auto assistant =
        assistant_srv_->RetrieveAssistantV2(create_dto.assistant_id);
    if (assistant.has_error()) {
      CTL_ERR("Failed to retrieve assistant: " + assistant.error());
      auto error = OpenAi::ErrorEvent(assistant.error());
      callback(error, true);
      return;
    }
  }).detach();
}
