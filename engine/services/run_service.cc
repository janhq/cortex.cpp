#include "run_service.h"
#include <thread>
#include "common/cortex/sync_queue.h"
#include "common/events/done.h"
#include "common/events/error.h"
#include "common/events/thread_message_delta.h"
#include "common/message_delta.h"
#include "utils/logging_utils.h"
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
      CTL_ERR("Failed to retrieve assistant: " + assistant.error());
      auto error = OpenAi::ErrorEvent(assistant.error());
      callback(error, true);
      return;
    }

    auto thread_res = thread_srv_->RetrieveThread(thread_id);
    if (thread_res.has_error()) {
      // TODO: namh will need to create new thread?
      auto error = OpenAi::ErrorEvent(thread_res.error());
      callback(error, true);
      return;
    }

    auto additional_messages = std::move(*create_dto.additional_messages);
    for (auto& msg : additional_messages) {
      auto create_msg_res = message_srv_->CreateMessage(
          thread_id, msg.role, std::move(msg.content),
          std::move(msg.attachments), std::move(msg.metadata));

      if (create_msg_res.has_error()) {
        CTL_WRN("Create message error: " + create_msg_res.error());
        continue;
      }
    }

    auto limit{-1};
    auto order{"desc"};
    auto after{""};
    auto before{""};
    auto run_id{""};
    auto messages_res = message_srv_->ListMessages(thread_id, limit, order,
                                                   after, before, run_id);
    // todo: recheck the order of messages

    auto model = create_dto.model.has_value() && !create_dto.model->empty()
                     ? create_dto.model.value()
                     : assistant->model;

    auto additional_inst = create_dto.additional_instructions.value_or("");
    auto instructions = create_dto.instructions.has_value()
                            ? create_dto.instructions.value()
                            : assistant->instructions.value_or("");
    instructions += ". " + additional_inst;

    auto temperature = create_dto.temperature.has_value()
                           ? create_dto.temperature.value()
                           : assistant->temperature.value_or(1.0f);

    auto top_p = create_dto.top_p.has_value() ? create_dto.top_p.value()
                                              : assistant->top_p.value_or(1.0f);
    auto stream{true};
    auto q = std::make_shared<SyncQueue>();
    Json::Value json;
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
    // TODO: namh should check if model is available or not

    json["messages"] = messages;
    json["model"] = model;
    json["stream"] = stream;

    auto json_ptr = std::make_shared<Json::Value>(std::move(json));

    auto ir = inference_srv_->HandleChatCompletion(q, json_ptr);
    if (ir.has_error()) {
      auto err = ir.error();
      auto error = OpenAi::ErrorEvent(std::get<1>(err).toStyledString());
      callback(error, true);
      return;
    }

    while (true) {
      auto [status, res] = q->wait_and_pop();

      if (status["has_error"].asBool()) {
        auto error = OpenAi::ErrorEvent(res["message"].asString());
        callback(error, true);
        return;
      }

      if (status["is_done"].asBool()) {
        auto done = OpenAi::DoneEvent();
        callback(done, true);
        return;
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

      auto content_str = json["choices"][0]["delta"]["content"].asString();

      auto text_ptr = std::make_unique<OpenAi::TextContent>();
      text_ptr->text = OpenAi::Text();
      text_ptr->text.value = std::move(content_str);

      auto content_list = std::vector<std::unique_ptr<OpenAi::Content>>();
      content_list.push_back(std::move(text_ptr));
      CTL_INF("Content list size: " + std::to_string(content_list.size()));

      auto delta = OpenAi::MessageDelta::Delta(OpenAi::Role::ASSISTANT,
                                               std::move(content_list));
      auto msg_delta_evt = OpenAi::ThreadMessageDeltaEvent(std::move(delta));
      callback(msg_delta_evt, false);
    }
  }).detach();
}
