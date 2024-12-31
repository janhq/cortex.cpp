#include "run_service.h"
#include <thread>
#include "common/cortex/sync_queue.h"
#include "common/events/done.h"
#include "common/events/error.h"
#include "common/events/thread_message_completed.h"
#include "common/events/thread_message_created.h"
#include "common/events/thread_message_delta.h"
#include "common/events/thread_message_in_progress.h"
#include "common/events/thread_run_completed.h"
#include "common/events/thread_run_created_event.h"
#include "common/events/thread_run_in_progress.h"
#include "common/events/thread_run_queued.h"
#include "common/message_delta.h"
#include "utils/logging_utils.h"
#include "utils/time_utils.h"
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
    dto::RunCreateDto&& create_dto, const std::string& thread_id,
    std::function<void(const OpenAi::AssistantStreamEvent&, bool)> callback)
    -> void {
  std::thread([this, &create_dto, thread_id, callback = std::move(callback)]() {
    auto assistant =
        assistant_srv_->RetrieveAssistantV2(create_dto.assistant_id);
    if (assistant.has_error()) {
      callback(OpenAi::ErrorEvent(assistant.error()), true);
      return;
    }

    auto thread = thread_srv_->RetrieveThread(thread_id);
    if (thread.has_error()) {
      callback(OpenAi::ErrorEvent(thread.error()), true);
      return;
    }

    auto model = create_dto.model.has_value() && !create_dto.model->empty()
                     ? create_dto.model.value()
                     : assistant->model;
    if (model.empty()) {
      callback(OpenAi::ErrorEvent(
                   "Model to use is empty. Please recheck the assistant"),
               true);
      return;
    }

    // TODO: check if engine and model is loaded yet

    auto additional_messages = std::move(*create_dto.additional_messages);
    for (auto& msg : additional_messages) {
      auto create_msg_res = message_srv_->CreateMessage(
          thread_id, msg.role, std::move(msg.content),
          std::move(msg.attachments), std::move(msg.metadata));

      if (create_msg_res.has_error()) {
        auto err_msg =
            "Create additional message error: " + create_msg_res.error();
        CTL_WRN(err_msg);
        callback(OpenAi::ErrorEvent(err_msg), true);
        return;
      }
    }

    auto additional_inst = create_dto.additional_instructions.value_or("");
    auto instructions = create_dto.instructions.has_value()
                            ? create_dto.instructions.value()
                            : assistant->instructions.value_or("");
    if (!instructions.empty() && !additional_inst.empty()) {
      instructions += ". " + additional_inst;
    } else if (instructions.empty()) {
      instructions = additional_inst;
    }

    auto temperature = create_dto.temperature.has_value()
                           ? create_dto.temperature.value()
                           : assistant->temperature.value_or(1.0f);

    auto top_p = create_dto.top_p.has_value() ? create_dto.top_p.value()
                                              : assistant->top_p.value_or(1.0f);

    auto request_json = GetMessageListAsJson(thread_id);
    if (request_json.has_error()) {
      callback(OpenAi::ErrorEvent(request_json.error()), true);
      return;
    }
    (*request_json.value())["model"] = model;
    (*request_json.value())["stream"] = true;
    (*request_json.value())["top_p"] = top_p;
    (*request_json.value())["temperature"] = temperature;

    auto id{"run_" + ulid::GenerateUlid()};

    OpenAi::Run run;
    run.id = id;
    run.object = "thread.run";
    run.created_at = cortex_utils::SecondsSinceEpoch();
    run.thread_id = thread_id;
    run.assistant_id = create_dto.assistant_id;
    run.status = OpenAi::RunStatus::QUEUED;
    run.model = model;
    run.instructions = instructions;
    run.tools = std::move(assistant->tools);
    run.metadata = create_dto.metadata.value_or(Cortex::VariantMap{});
    run.temperature = temperature;
    run.top_p = top_p;
    run.max_prompt_tokens = create_dto.max_prompt_tokens;
    run.max_completion_tokens = create_dto.max_completion_tokens;
    auto truncation_strategy{OpenAi::TruncationStrategy()};
    if (create_dto.truncation_strategy) {
      truncation_strategy = std::move(create_dto.truncation_strategy.value());
    }
    run.truncation_strategy = std::move(truncation_strategy);
    if (create_dto.tool_choice) {
      auto& choice = create_dto.tool_choice.value();
      if (std::holds_alternative<std::string>(choice)) {
        run.tool_choice = std::get<std::string>(choice);
      } else {
        run.tool_choice = std::get<OpenAi::ToolChoice>(choice);
      }
    } else {
      run.tool_choice = std::string("none");
    }
    run.parallel_tool_calls = create_dto.parallel_tool_calls.value_or(true);
    if (create_dto.response_format) {
      run.response_format = std::move(create_dto.response_format.value());
    } else {
      run.response_format = std::string("auto");
    }

    {
      auto res = run_repository_->CreateRun(run);
      if (res.has_error()) {
        callback(OpenAi::ErrorEvent(res.error()), true);
        return;
      }
      callback(OpenAi::ThreadRunCreatedEvent(
                   run.ToSingleLineJsonString(false).value()),
               false);
    }
    callback(
        OpenAi::ThreadRunQueuedEvent(run.ToSingleLineJsonString(false).value()),
        false);

    callback(OpenAi::ThreadRunInProgressEvent(
                 run.ToSingleLineJsonString(false).value()),
             false);
    auto q = std::make_shared<SyncQueue>();
    {
      auto ir = inference_srv_->HandleChatCompletion(
          q, std::shared_ptr<Json::Value>(std::move(request_json.value())));
      if (ir.has_error()) {
        auto error =
            OpenAi::ErrorEvent(std::get<1>(ir.error()).toStyledString());
        callback(error, true);
        return;
      }
    }

    auto assistant_msg = message_srv_->CreateMessage(
        thread_id, OpenAi::Role::ASSISTANT, "", std::nullopt, std::nullopt,
        OpenAi::Status::IN_PROGRESS);
    if (assistant_msg.has_error()) {
      callback(OpenAi::ErrorEvent(assistant_msg.error()), true);
      return;
    }

    {
      assistant_msg->assistant_id = assistant->id;
      assistant_msg->run_id = id;
      auto res = message_srv_->ModifyMessage(assistant_msg.value());
      if (res.has_error()) {
        CTL_ERR("Failed to modify message: " + res.error());
      }
      callback(OpenAi::ThreadMessageCreatedEvent(
                   assistant_msg->ToSingleLineJsonString(false).value()),
               false);
    }

    callback(OpenAi::ThreadMessageInProgressEvent(
                 assistant_msg->ToSingleLineJsonString(false).value()),
             false);

    while (true) {
      auto [status, res] = q->wait_and_pop();

      if (status["has_error"].asBool()) {
        auto err_msg{res["message"].asString()};
        {
          assistant_msg->incomplete_details = OpenAi::IncompleteDetail(err_msg);
          assistant_msg->incomplete_at = cortex_utils::SecondsSinceEpoch();
          assistant_msg->status = OpenAi::Status::INCOMPLETE;
          auto res = message_srv_->ModifyMessage(assistant_msg.value());
          if (res.has_error()) {
            CTL_ERR("Failed to modify message: " + res.error());
          }
        }
        callback(OpenAi::ErrorEvent(err_msg), true);
        return;
      }

      if (status["is_done"].asBool()) {
        {
          assistant_msg->completed_at = cortex_utils::SecondsSinceEpoch();
          assistant_msg->status = OpenAi::Status::COMPLETED;
          auto res = message_srv_->ModifyMessage(assistant_msg.value());
          if (res.has_error()) {
            CTL_ERR("Failed to modify message: " + res.error());
          }
          callback(OpenAi::ThreadMessageCompletedEvent(
                       assistant_msg->ToSingleLineJsonString(false).value()),
                   false);
        }
        break;
      }

      auto str = res["data"].asString();
      if (str.substr(0, 6) == "data: ") {
        str = str.substr(6);
      }
      Json::Value json;
      Json::Reader reader;
      bool parse_success = reader.parse(str, json);
      if (!parse_success) {
        CTL_ERR("Failed to parse JSON: " + reader.getFormattedErrorMessages());
        continue;
      }

      if (!json.isMember("choices") || json["choices"].empty() ||
          !json["choices"][0].isMember("delta") ||
          !json["choices"][0]["delta"].isMember("content")) {
        CTL_WRN("Missing required fields in JSON");
        CTL_WRN("Has choices: " + std::to_string(json.isMember("choices")));
        if (json.isMember("choices")) {
          CTL_WRN("Choices size: " + std::to_string(json["choices"].size()));
          CTL_WRN("First choice has delta: " +
                  std::to_string(json["choices"][0].isMember("delta")));
        }
        continue;
      }

      {
        auto text_content = std::make_unique<OpenAi::TextContent>(
            json["choices"][0]["delta"]["content"].asString());

        auto content = std::vector<std::unique_ptr<OpenAi::Content>>();
        content.push_back(std::move(text_content));
        // Get existing content or create new if empty
        if (!assistant_msg->content.empty()) {
          // Find the last text content to append to
          for (auto it = assistant_msg->content.rbegin();
               it != assistant_msg->content.rend(); ++it) {
            if (auto* text_content =
                    dynamic_cast<OpenAi::TextContent*>(it->get())) {
              // Append the new delta text to existing text content
              text_content->text.value +=
                  json["choices"][0]["delta"]["content"].asString();
              break;
            }
          }
        } else {
          // Create new text content if message is empty
          auto text_content = std::make_unique<OpenAi::TextContent>(
              json["choices"][0]["delta"]["content"].asString());
          assistant_msg->content.push_back(std::move(text_content));
        }

        // Update message
        assistant_msg->status = OpenAi::Status::IN_PROGRESS;
        auto res = message_srv_->ModifyMessage(assistant_msg.value());
        if (res.has_error()) {
          CTL_ERR("Failed to modify message: " + res.error());
        }
      }

      auto text_content = std::make_unique<OpenAi::TextContent>(
          json["choices"][0]["delta"]["content"].asString());

      auto content = std::vector<std::unique_ptr<OpenAi::Content>>();
      content.push_back(std::move(text_content));

      auto delta = OpenAi::MessageDelta::Delta(OpenAi::Role::ASSISTANT,
                                               std::move(content));
      auto msg_delta_evt = OpenAi::ThreadMessageDeltaEvent(std::move(delta));
      callback(msg_delta_evt, false);
    }

    run.status = OpenAi::RunStatus::COMPLETED;
    run.completed_at = cortex_utils::SecondsSinceEpoch();
    callback(OpenAi::ThreadRunCompletedEvent(
                 run.ToSingleLineJsonString(false).value()),
             false);

    callback(OpenAi::DoneEvent(), true);
  }).detach();
}

auto RunService::GetMessageListAsJson(const std::string& thread_id)
    -> cpp::result<std::unique_ptr<Json::Value>, std::string> {
  auto messages_res =
      message_srv_->ListMessages(thread_id, -1, "asc", "", "", "");
  if (messages_res.has_error()) {
    return cpp::fail(messages_res.error());
  }

  Json::Value messages(Json::arrayValue);
  // TODO: namh check what if the message is not text based?
  // TODO: namh what if message have multiple text array item?
  for (const auto& msg : messages_res.value()) {
    Json::Value message;
    for (const auto& content : msg.content) {
      if (content->type == "text") {
        if (auto* text_content =
                dynamic_cast<OpenAi::TextContent*>(content.get())) {
          auto text = text_content->text.value;
          message["content"] = std::move(text);
          break;
        }
      }
    }

    message["role"] = RoleToString(msg.role);
    messages.append(message);
  }

  auto json = std::make_unique<Json::Value>();
  (*json)["messages"] = std::move(messages);
  CTL_INF("GetMessageListAsJson: " + json->toStyledString());
  return json;
}

auto RunService::ListRuns(const std::string& thread_id, uint8_t limit,
                          const std::string& order, const std::string& after,
                          const std::string& before) const
    -> cpp::result<std::vector<OpenAi::Run>, std::string> {
  return run_repository_->ListRuns(limit, order, after, before);
}

auto RunService::RetrieveRun(const std::string& thread_id,
                             const std::string& run_id) const
    -> cpp::result<OpenAi::Run, std::string> {
  return run_repository_->RetrieveRun(run_id);
}
