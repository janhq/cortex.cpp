#pragma once

#include "common/api_server_configuration.h"
#include "utils/result.hpp"

class ConfigService {
 public:
  cpp::result<ApiServerConfiguration, std::string> UpdateApiServerConfiguration(
      const Json::Value& json);

  cpp::result<ApiServerConfiguration, std::string> GetApiServerConfiguration()
      const;
};
