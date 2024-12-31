#include "process_manager.h"
#include "utils/cortex_utils.h"

void ProcessManager::destroy(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto loaded_engines = engine_service_->GetSupportedEngineNames();
  for (const auto& engine : loaded_engines.value()) {
    auto res = engine_service_->UnloadEngine(engine);
    if (res.has_error()) {
      CTL_WRN("Failed to unload engine: " + res.error());
    }
  }
  app().quit();
  Json::Value ret;
  ret["message"] = "Program is exitting, goodbye!";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
  LOG_INFO << "Program is exitting, goodbye!";
};
