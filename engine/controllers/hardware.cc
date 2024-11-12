#include "hardware.h"
#include "common/hardware_config.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/scope_exit.h"

void Hardware::GetHardwareInfo(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto hw_inf = hw_svc_->GetHardwareInfo();
  Json::Value ret;
  ret["cpu"] = hardware::ToJson(hw_inf.cpu);
  ret["os"] = hardware::ToJson(hw_inf.os);
  ret["ram"] = hardware::ToJson(hw_inf.ram);
  ret["storage"] = hardware::ToJson(hw_inf.storage);
  ret["gpus"] = hardware::ToJson(hw_inf.gpus);
  ret["power"] = hardware::ToJson(hw_inf.power);
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
}

void Hardware::Activate(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
#if defined(__APPLE__) && defined(__MACH__)
  Json::Value ret;
  ret["message"] = "Item requested was not found";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k400BadRequest);
  callback(resp);
#else
  // {
  //   "gpus" : [0, 1]
  // }
  cortex::hw::ActivateHardwareConfig ahc;
  if (auto o = req->getJsonObject(); o) {
    CTL_INF("activate: " << o->toStyledString());
    for (auto& g : (*o)["gpus"]) {
      ahc.gpus.push_back(g.asInt());
    }
  }
  std::sort(ahc.gpus.begin(), ahc.gpus.end());
  if (!hw_svc_->IsValidConfig(ahc)) {
    Json::Value ret;
    ret["message"] = "Invalid GPU index provided.";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
    return;
  };

  if (!hw_svc_->SetActivateHardwareConfig(ahc)) {
    Json::Value ret;
    ret["message"] = "The hardware configuration is already up to date.";
    auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
    resp->setStatusCode(k200OK);
    callback(resp);
    return;
  }

  if (auto r = engine_svc_->UnloadEngine(kLlamaEngine); r.has_error()) {
    CTL_WRN(r.error());
  }

  Json::Value ret;
  ret["message"] = "The hardware configuration has been activated.";
  if (auto o = req->getJsonObject(); o) {
    ret["activated_gpus"] = (*o)["gpus"];
  }
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);
  app().quit();
#endif
}