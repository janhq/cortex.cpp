#pragma once

#include <drogon/HttpController.h>
#include "services/hardware_service.h"

using namespace drogon;

class Hardware : public drogon::HttpController<Hardware, false> {
 public:
  METHOD_LIST_BEGIN
  METHOD_ADD(Hardware::GetHardwareInfo, "/hardware", Get);

  ADD_METHOD_TO(Hardware::GetHardwareInfo, "/v1/hardware", Get);
  METHOD_LIST_END

  void GetHardwareInfo(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback);

 private:
  services::HardwareService hw_svc_;
};