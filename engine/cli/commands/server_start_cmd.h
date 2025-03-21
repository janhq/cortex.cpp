#pragma once

#include <optional>
#include <string>
#include "utils/config_yaml_utils.h"
#include "utils/curl_utils.h"
#include "utils/logging_utils.h"
#include "utils/url_parser.h"

namespace commands {
using CortexConfig = config_yaml_utils::CortexConfig;

inline bool IsServerAlive(const std::string& host, int port) {
  auto url = url_parser::Url{
      /* .protocol = */ "http",
      /* .host = */ host + ":" + std::to_string(port),
      /* .pathParams = */ {"healthz"},
      /* .queries = */ {},
  };
  auto res = curl_utils::SimpleGet(url.ToFullPath());
  if (res.has_error()) {
    CTL_WRN("Server is not alive: " << res.error());
    return false;
  }
  return true;
}

class ServerStartCmd {
 public:
  bool Exec(const std::string& host, int port,
            const std::optional<std::string>& log_level = std::nullopt);

  bool Exec(const std::optional<std::string>& log_level,
            const std::unordered_map<std::string, std::string>& options,
            CortexConfig& data);

 private:
  void UpdateConfig(CortexConfig& data, const std::string& key,
                    const std::string& value);

  void UpdateVectorField(
      const std::string& key, const std::string& value,
      std::function<void(const std::vector<std::string>&)> setter);

  void UpdateNumericField(const std::string& key, const std::string& value,
                          std::function<void(float)> setter);

  void UpdateBooleanField(const std::string& key, const std::string& value,
                          std::function<void(bool)> setter);
};
}  // namespace commands
