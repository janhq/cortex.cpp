#pragma once

#include <drogon/HttpController.h>
#include "common/engine_servicei.h"
#include "services/hardware_service.h"

using namespace drogon;

class Hardware : public drogon::HttpController<Hardware, false> {
 public:
  explicit Hardware(std::shared_ptr<EngineServiceI> engine_svc,
                    std::shared_ptr<HardwareService> hw_svc)
      : engine_svc_(engine_svc), hw_svc_(hw_svc) {}
  METHOD_LIST_BEGIN
  METHOD_ADD(Hardware::GetHardwareInfo, "/hardware", Get);
  METHOD_ADD(Hardware::Activate, "/hardware/activate", Post);

  ADD_METHOD_TO(Hardware::GetHardwareInfo, "/v1/hardware", Get);
  ADD_METHOD_TO(Hardware::Activate, "/v1/hardware/activate", Post);
  METHOD_LIST_END

  void GetHardwareInfo(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback);

  void Activate(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback);

 private:
  std::shared_ptr<EngineServiceI> engine_svc_ = nullptr;
  std::shared_ptr<HardwareService> hw_svc_= nullptr;
};