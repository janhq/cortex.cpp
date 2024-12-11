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