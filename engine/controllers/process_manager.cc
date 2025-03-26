#include "process_manager.h"
#include <trantor/utils/Logger.h>
#include <cstdlib>
#include "json/json.h"
#include "utils/cortex_utils.h"

void ProcessManager::destroy(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  (void)req;
  auto loaded_engines = engine_service_->GetSupportedEngineNames();
  for (const auto& engine : loaded_engines.value()) {
    auto result = engine_service_->UnloadEngine(engine);
    if (!result) {
      // Handle the error if any.
      // Log the Error
      LOG_ERROR << "Error unloading engine: " << result.error();
      continue;
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
