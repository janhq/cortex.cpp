#pragma once

#include "remote_engine.h"

namespace remote_engine {
class OpenAiEngine : public RemoteEngine {
 public:
  void GetModels(
      std::shared_ptr<Json::Value> json_body,
      std::function<void(Json::Value&&, Json::Value&&)>&& callback) override;

  Json::Value GetRemoteModels() override;
};
}  // namespace remote_engine