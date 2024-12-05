#include "anthropic_engine.h"
#include <array>
#include <string_view>
#include "utils/logging_utils.h"

namespace remote_engine {
namespace {
constexpr const std::array<std::string_view, 5> kAnthropicModels = {
    "claude-3-5-sonnet-20241022", "claude-3-5-haiku-20241022",
    "claude-3-opus-20240229", "claude-3-sonnet-20240229",
    "claude-3-haiku-20240307"};
}
void AnthropicEngine::GetModels(
    std::shared_ptr<Json::Value> json_body,
    std::function<void(Json::Value&&, Json::Value&&)>&& callback) {
  Json::Value json_resp;
  Json::Value model_array(Json::arrayValue);
  {
    std::shared_lock l(models_mtx_);
    for (const auto& [m, _] : models_) {
      Json::Value val;
      val["id"] = m;
      val["engine"] = "anthropic";
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

Json::Value AnthropicEngine::GetRemoteModels() {
  Json::Value json_resp;
  Json::Value model_array(Json::arrayValue);
  for (const auto& m : kAnthropicModels) {
    Json::Value val;
    val["id"] = std::string(m);
    val["engine"] = "anthropic";
    val["created"] = "_";
    val["object"] = "model";
    model_array.append(val);
  }

  json_resp["object"] = "list";
  json_resp["data"] = model_array;
  CTL_INF("Remote models responded");
  return json_resp;
}
}  // namespace remote_engine