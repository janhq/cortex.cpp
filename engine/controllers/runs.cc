#include "runs.h"
#include <memory>
#include "common/events/assistant_stream_event.h"
#include "utils/cortex_utils.h"

void Runs::CreateRun(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& thread_id) {
  auto json_body = req->getJsonObject();
  if (json_body == nullptr) {
    Json::Value ret;
    ret["message"] = "Body can't be empty";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  auto run_create_dto = dto::RunCreateDto::FromJson(std::move(*json_body));
  if (run_create_dto.has_error()) {
    Json::Value ret;
    ret["message"] = run_create_dto.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  struct SharedState {
    dto::RunCreateDto dto;
    std::string thread_id;
    std::shared_ptr<drogon::ResponseStream> stream;

    explicit SharedState(dto::RunCreateDto&& d) : dto(std::move(d)) {}
  };

  auto state = std::make_shared<SharedState>(std::move(run_create_dto.value()));
  state->thread_id = thread_id;

  auto sendEvents = [this, state](ResponseStreamPtr res) {
    state->stream = std::shared_ptr<drogon::ResponseStream>(std::move(res));

    run_srv_->CreateRunStream(
        std::move(state->dto), state->thread_id,
        [state](const OpenAi::AssistantStreamEvent& event, bool disconnect) {
          auto parse_event_res = event.ToEvent();
          if (parse_event_res.has_value()) {
            state->stream->send(parse_event_res.value());
            if (disconnect) {
              state->stream->close();
            }
          }
        });
  };

  // TODO: namh add the date time
  auto resp = HttpResponse::newAsyncStreamResponse(std::move(sendEvents));
  resp->setContentTypeString("text/event-stream");
  resp->addHeader("Cache-Control", "no-cache");
  resp->addHeader("Connection", "keep-alive");
  callback(resp);
}

void Runs::RetrieveRun(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& thread_id,
                       const std::string& run_id) {
  auto res = run_srv_->RetrieveRun(thread_id, run_id);
  if (res.has_error()) {
    Json::Value ret;
    ret["message"] = res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
  } else {
    auto json = res->ToJson();
    if (json.has_error()) {
      CTL_ERR("Failed to convert run to json: " + json.error());
      Json::Value ret;
      ret["message"] = json.error();
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
      resp->setStatusCode(k400BadRequest);
      callback(resp);
    } else {
      auto resp = cortex_utils::CreateCortexHttpJsonResponse(json.value());
      resp->setStatusCode(k200OK);
      callback(resp);
    }
  }
}

void Runs::CancelRun(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& thread_id, const std::string& run_id) {}

void Runs::ModifyRun(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback,
                     const std::string& thread_id, const std::string& run_id) {}

void Runs::SubmitToolOutput(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    const std::string& thread_id, const std::string& run_id) {}

void Runs::ListRuns(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    const std::string& thread_id,
                    std::optional<std::string> limit,
                    std::optional<std::string> order,
                    std::optional<std::string> after,
                    std::optional<std::string> before) const {
  auto res = run_srv_->ListRuns(thread_id, std::stoi(limit.value_or("20")),
                                order.value_or("desc"), after.value_or(""),
                                before.value_or(""));

  if (res.has_error()) {
    Json::Value root;
    root["message"] = res.error();
    auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
    response->setStatusCode(k400BadRequest);
    callback(response);
    return;
  }
  Json::Value msg_arr(Json::arrayValue);
  for (auto& msg : res.value()) {
    if (auto it = msg.ToJson(); it.has_value()) {
      msg_arr.append(it.value());
    } else {
      CTL_WRN("Failed to convert run to json: " + it.error());
    }
  }

  Json::Value root;
  root["object"] = "list";
  root["data"] = msg_arr;
  auto response = cortex_utils::CreateCortexHttpJsonResponse(root);
  response->setStatusCode(k200OK);
  callback(response);
}
