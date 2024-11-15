#include "config_service.h"
#include "common/api_server_configuration.h"
#include "utils/file_manager_utils.h"

cpp::result<ApiServerConfiguration, std::string>
ConfigService::UpdateApiServerConfiguration(const Json::Value& json) {
  auto config = file_manager_utils::GetCortexConfig();
  ApiServerConfiguration api_server_config{
      config.enableCors, config.allowedOrigins, config.verifyProxySsl,
      config.proxyUrl};
  std::vector<std::string> updated_fields;
  std::vector<std::string> invalid_fields;
  std::vector<std::string> unknown_fields;

  api_server_config.UpdateFromJson(json, &updated_fields, &invalid_fields,
                                   &unknown_fields);

  if (updated_fields.empty()) {
    return cpp::fail("No configuration updated");
  }

  config.enableCors = api_server_config.cors;
  config.allowedOrigins = api_server_config.allowed_origins;
  config.proxyUrl = api_server_config.proxy_url;
  config.verifyProxySsl = api_server_config.verify_proxy_ssl;

  auto result = file_manager_utils::UpdateCortexConfig(config);
  return api_server_config;
}

cpp::result<ApiServerConfiguration, std::string>
ConfigService::GetApiServerConfiguration() {
  auto config = file_manager_utils::GetCortexConfig();
  // todo: namh support username and password
  return ApiServerConfiguration{config.enableCors, config.allowedOrigins,
                                config.verifyProxySsl, config.proxyUrl};
}
