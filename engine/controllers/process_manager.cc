#include "process_manager.h"
#include <trantor/utils/Logger.h>
#include <cstdlib>
#include "json/json.h"
#include "utils/cortex_utils.h"

void ProcessManager::destroy(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto res = inference_service_->GetModels(std::make_shared<Json::Value>());
  // Check if there are any running models
  auto running_models = std::get<1>(res);
  if (!running_models.get("data", Json::Value()).empty()) {
    for (const auto& model : running_models["data"]) {
      std::string model_id = model["id"].asString();
      model_service_->StopModel(model_id);
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
