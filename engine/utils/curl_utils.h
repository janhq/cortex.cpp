#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "yaml-cpp/yaml.h"

namespace curl_utils {
namespace {
size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     std::string* output) {
  size_t totalSize = size * nmemb;
  output->append((char*)contents, totalSize);
  return totalSize;
}
}  // namespace

inline cpp::result<std::string, std::string> SimpleGet(const std::string& url) {
  CURL* curl;
  CURLcode res;
  std::string readBuffer;

  // Initialize libcurl
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (!curl) {
    return cpp::fail("Failed to init CURL");
  }
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  // Set write function callback and data buffer
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

  // Perform the request
  res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    return cpp::fail("CURL request failed: " +
                     static_cast<std::string>(curl_easy_strerror(res)));
  }

  curl_easy_cleanup(curl);
  return readBuffer;
}

inline cpp::result<YAML::Node, std::string> ReadRemoteYaml(
    const std::string& url) {
  auto result = SimpleGet(url);
  if (result.has_error()) {
    return cpp::fail(result.error());
  }

  try {
    return YAML::Load(result.value());
  } catch (const std::exception& e) {
    return cpp::fail("YAML from " + url +
                     " parsing error: " + std::string(e.what()));
  }
}

inline cpp::result<nlohmann::json, std::string> SimpleGetJson(
    const std::string& url) {
  auto result = SimpleGet(url);
  if (result.has_error()) {
    return cpp::fail(result.error());
  }

  try {
    return nlohmann::json::parse(result.value());
  } catch (const std::exception& e) {
    return cpp::fail("JSON from " + url +
                     " parsing error: " + std::string(e.what()));
  }
}
}  // namespace curl_utils