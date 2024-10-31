#include "hardware.h"
#include "utils/cortex_utils.h"

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