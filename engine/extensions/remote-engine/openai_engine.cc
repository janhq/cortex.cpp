#include "openai_engine.h"
#include "utils/logging_utils.h"

namespace remote_engine {

void OpenAiEngine::GetModels(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  Json::Value json_resp;
  Json::Value model_array(Json::arrayValue);
  {
    std::shared_lock l(models_mtx_);
    for (const auto& [m, _] : models_) {
      Json::Value val;
      val["id"] = m;
      val["engine"] = "openai";
      val["start_time"] = "_";
      val["model_size"] = "_";
      val["vram"] = "_";
      val["ram"] = "_";
      val["object"] = "model";
      model_array.append(val);
    }
  }

  json_resp["object"] = "list";
  json_resp["data"] = model_array;

  Json::Value status;
  status["is_done"] = true;
  status["has_error"] = false;
  status["is_stream"] = false;
  status["status_code"] = 200;
  callback(std::move(status), std::move(json_resp));
  CTL_INF("Running models responded");
}

Json::Value OpenAiEngine::GetRemoteModels() {
  auto response = MakeGetModelsRequest();
  if (response.error) {
    Json::Value error;
    error["error"] = response.error_message;
    return error;
  }
  Json::Value response_json;
  Json::Reader reader;
  if (!reader.parse(response.body, response_json)) {
    Json::Value error;
    error["error"] = "Failed to parse response";
    return error;
  }
  return response_json;
}
}  // namespace remote_engine