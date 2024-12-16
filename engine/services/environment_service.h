#pragma once
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "utils/environment_utils.h"
#include "utils/system_info_utils.h"

using Environment = environment_utils::Environment;

struct EnvironmentsUpdateResult {
  Environment environment;
  std::string from;
  std::string to;

  Json::Value ToJson() const {
    Json::Value root;
    root["environment"] = environment.ToJson();
    root["from"] = from;
    root["to"] = to;
    return root;
  }
};

class EnvironmentService {
 public:
  cpp::result<bool, std::string> IsEnvironmentReady(
      const std::string& environment);
  cpp::result<void, std::string> InstallEnvironmentAsync(
      const std::string& environment, const std::string& version);
  cpp::result<void, std::string> UnInstallEnvironment(
      const std::string& environment, const std::string& version);
  cpp::result<std::vector<Environment>, std::string> GetEnvironmentReleases(
      const std::string& environment) const;
  cpp::result<std::vector<Environment>, std::string> GetInstalledEnvironments()
      const;
  cpp::result<std::vector<Environment>, std::string> GetDefaultEnvironment(
      const std::string& environment) const;
  cpp::result<std::vector<Environment>, std::string> SetDefaultEnvironment(
      const std::string& environment) const;

 private:
  cpp::result<void, std::string> DownloadEnvironment(
      const std::string& environment, const std::string& version = "latest");
  cpp::result<std::pair<std::filesystem::path, bool>, std::string>
  GetEnvironmentDirPath(const std::string& environment);
};