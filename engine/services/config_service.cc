#include "config_service.h"
#include "common/api_server_configuration.h"
#include "utils/file_manager_utils.h"

cpp::result<ApiServerConfiguration, std::string>
ConfigService::UpdateApiServerConfiguration(const Json::Value& json) {
  auto config = file_manager_utils::GetCortexConfig();
  ApiServerConfiguration api_server_config{
      config.enableCors,         config.allowedOrigins,   config.verifyProxySsl,
      config.verifyProxyHostSsl, config.proxyUrl,         config.proxyUsername,
      config.proxyPassword,      config.noProxy,          config.verifyPeerSsl,
      config.verifyHostSsl,      config.huggingFaceToken, config.gitHubToken};

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
  config.verifyProxySsl = api_server_config.verify_proxy_ssl;
  config.verifyProxyHostSsl = api_server_config.verify_proxy_host_ssl;

  config.proxyUrl = api_server_config.proxy_url;
  config.proxyUsername = api_server_config.proxy_username;
  config.proxyPassword = api_server_config.proxy_password;
  config.noProxy = api_server_config.no_proxy;

  config.verifyPeerSsl = api_server_config.verify_peer_ssl;
  config.verifyHostSsl = api_server_config.verify_host_ssl;

  config.huggingFaceToken = api_server_config.hf_token;
  config.gitHubToken = api_server_config.gh_token;

  auto result = file_manager_utils::UpdateCortexConfig(config);
  return api_server_config;
}

cpp::result<ApiServerConfiguration, std::string>
ConfigService::GetApiServerConfiguration() {
  auto config = file_manager_utils::GetCortexConfig();
  return ApiServerConfiguration{
      config.enableCors,         config.allowedOrigins,   config.verifyProxySsl,
      config.verifyProxyHostSsl, config.proxyUrl,         config.proxyUsername,
      config.proxyPassword,      config.noProxy,          config.verifyPeerSsl,
      config.verifyHostSsl,      config.huggingFaceToken, config.gitHubToken};
}
