#include "process_manager.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

#include <trantor/utils/Logger.h>
#include <cstdlib>

void ProcessManager::destroy(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

  auto destroy_result = download_service_->Destroy();
  if (destroy_result.has_error()) {
    CTL_ERR("Failed to destroy download service: " + destroy_result.error());
  } else {
    CTL_INF("Download service stopped!");
  }

  app().quit();
  Json::Value ret;
  ret["message"] = "Program is exitting, goodbye!";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
  LOG_INFO << "Program is exitting, goodbye!";
};
