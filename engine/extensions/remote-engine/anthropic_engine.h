#pragma once
#include "remote_engine.h"

namespace remote_engine {
class AnthropicEngine : public RemoteEngine {
 public:

  Json::Value GetRemoteModels() override;
};
}  // namespace remote_engine