#pragma once
#include <json/json.h>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>
#include "utils/curl_utils.h"
#include "utils/result.hpp"
namespace environment_utils {

constexpr const auto kBaseEnvironmentsUrl =
    "https://delta.jan.ai/environments/";

struct Environment {
  std::string type;     // e.g., "python"
  std::string name;     // e.g., "whispervq"
  std::string version;  // e.g., "latest"
  std::string os;       // e.g., "window", "linux"
  std::string arch;     // e.g., "amd64"

  // Convert Environment to JSON
  Json::Value ToJson() const {
    Json::Value json;
    json["type"] = type;
    json["name"] = name;
    json["version"] = version;
    json["os"] = os;
    json["arch"] = arch;
    return json;
  }

  // Create Environment from JSON
  static cpp::result<Environment, std::string> FromJson(
      const Json::Value& json) {
    Environment env;

    // Validate required fields
    const std::vector<std::string> required_fields = {"type", "name", "version",
                                                      "os", "arch"};

    for (const auto& field : required_fields) {
      if (!json.isMember(field) || json[field].asString().empty()) {
        return cpp::fail("Missing or empty required field: " + field);
      }
    }

    env.type = json["type"].asString();
    env.name = json["name"].asString();
    env.version = json["version"].asString();
    env.os = json["os"].asString();
    env.arch = json["arch"].asString();

    return env;
  }

  // Method to generate full artifact URL
  std::string generateUrl() const {
    return kBaseEnvironmentsUrl + type + "/" + name + "/" + version + "/" +
           name + "-" + os + "-" + arch + ".zip";
  }

  // Method to validate the environment structure
  bool isValid() const {
    return !type.empty() && !name.empty() && !version.empty() && !os.empty() &&
           !arch.empty();
  }
};

// Utility function to parse URL components into an Environment struct
cpp::result<Environment, std::string> parseEnvironmentUrl(
    const std::string& url) {
  Environment env;

  size_t environments_pos = url.find("environments/");
  if (environments_pos == std::string::npos) {
    return cpp::fail("Invalid URL format");
  }

  std::string remaining = url.substr(environments_pos + 13);
  std::vector<std::string> parts;
  size_t pos = 0;
  while ((pos = remaining.find('/')) != std::string::npos) {
    parts.push_back(remaining.substr(0, pos));
    remaining.erase(0, pos + 1);
  }
  parts.push_back(remaining);

  if (parts.size() < 5) {
    return cpp::fail("Insufficient URL components");
  }

  env.type = parts[0];
  env.name = parts[1];
  env.version = parts[2];

  // Extract OS and arch from the filename
  std::string filename = parts[3];
  size_t os_sep = filename.find('-');
  size_t arch_sep = filename.find('-', os_sep + 1);

  if (os_sep == std::string::npos || arch_sep == std::string::npos) {
    return cpp::fail("Cannot parse OS and architecture");
  }

  env.os = filename.substr(os_sep + 1, arch_sep - os_sep - 1);
  env.arch = filename.substr(arch_sep + 1, filename.find('.') - arch_sep - 1);

  return env;
}

// Fetch environment names
cpp::result<std::vector<std::string>, std::string> fetchEnvironmentNames(
    const std::string& type, int timeout = 30) {
  auto url = kBaseEnvironmentsUrl + type;
  auto json_result = curl_utils::SimpleGetJson(url, timeout, false);
  if (json_result.has_error()) {
    return cpp::fail(json_result.error());
  }

  std::vector<std::string> environment_names;
  const Json::Value& root = json_result.value();

  // Store unique environment names
  std::unordered_set<std::string> unique_names;

  for (const auto& item : root) {
    if (item.isMember("path")) {
      environment_names.push_back(item["path"].asString());
    }
  }

  return environment_names;
}

// Get all versions for a specific environment
cpp::result<std::vector<std::string>, std::string> fetchEnvironmentVersions(
    const std::string& base_url, const std::string& environment_name,
    int timeout = 30, bool recursive = true) {
  auto json_result = curl_utils::SimpleGetJson(
      base_url + "/" + environment_name, timeout, recursive);
  if (json_result.has_error()) {
    return cpp::fail(json_result.error());
  }

  std::vector<std::string> versions;
  const Json::Value& root = json_result.value();

  // Store unique versions
  std::unordered_set<std::string> unique_versions;

  for (const auto& item : root) {
    if (item.isMember("path")) {
      auto url_parse_result = parseEnvironmentUrl(
          base_url + "/" + environment_name + "/" + item["path"].asString());
      if (!url_parse_result.has_error()) {
        const auto& env = url_parse_result.value();
        // Only add if not already present
        if (unique_versions.insert(env.version).second) {
          versions.push_back(env.version);
        }
      }
    }
  }

  return versions;
}

}  // namespace environment_utils