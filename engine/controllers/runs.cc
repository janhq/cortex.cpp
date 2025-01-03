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

  auto parse_run_res = dto::RunCreateDto::FromJson(std::move(*json_body));
  if (parse_run_res.has_error()) {
    Json::Value ret;
    ret["message"] = parse_run_res.error();
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  }

  // Create a shared state structure to hold our resources
  struct SharedState {
    dto::RunCreateDto dto;
    std::string thread_id;
    std::shared_ptr<drogon::ResponseStream> stream;
    explicit SharedState(dto::RunCreateDto&& d) : dto(std::move(d)) {}
  };

  // Create shared state with move-constructed DTO
  auto state = std::make_shared<SharedState>(std::move(parse_run_res.value()));
  state->thread_id = thread_id;

  auto sendEvents = [this, state](ResponseStreamPtr res) {
    // Store the response stream in the shared state
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

  auto resp = HttpResponse::newAsyncStreamResponse(std::move(sendEvents));
  resp->setContentTypeString("text/event-stream");
  resp->addHeader("Cache-Control", "no-cache");
  resp->addHeader("Connection", "keep-alive");
  callback(resp);
}

void Runs::RetrieveRun(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       const std::string& thread_id,
                       const std::string& run_id) {}

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
