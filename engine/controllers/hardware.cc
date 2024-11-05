#include "hardware.h"
#include "utils/cortex_utils.h"
#include "utils/file_manager_utils.h"
#include "utils/scope_exit.h"

void Hardware::GetHardwareInfo(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {
  auto hw_inf = hw_svc_.GetHardwareInfo();
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
  app().quit();
  Json::Value ret;
  ret["message"] = "Done";
  auto resp = cortex_utils::CreateCortexHttpJsonResponse(ret);
  resp->setStatusCode(k200OK);
  callback(resp);

  LOG_INFO << "Restarting...";
  // {
  //   "gpus" : [0, 1]
  // }
  services::ActivateHardwareConfig ahc;
  if (auto o = req->getJsonObject(); o) {
    for (auto& g : (*o)["gpus"]) {
      ahc.gpus.push_back(g.asInt());
    }
  }

  auto config = file_manager_utils::GetCortexConfig();
  hw_svc_.Restart(config.apiServerHost, std::stoi(config.apiServerPort), ahc);
}