#pragma once

#include <curl/curl.h>
#include <json/reader.h>
#include <json/value.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <string>
#include "utils/result.hpp"

namespace curl_utils {
namespace {
size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     std::string* output) {
  size_t totalSize = size * nmemb;
  output->append((char*)contents, totalSize);
  return totalSize;
}
}  // namespace

inline cpp::result<std::string, std::string> SimpleGet(
    const std::string& url,
    const std::optional<std::reference_wrapper<
        const std::unordered_map<std::string, std::string>>>& headers =
        std::nullopt) {

  // Initialize libcurl
  curl_global_init(CURL_GLOBAL_DEFAULT);
  auto curl = curl_easy_init();

  if (!curl) {
    return cpp::fail("Failed to init CURL");
  }

  if (headers.has_value()) {
    curl_slist* curl_headers = nullptr;

    for (const auto& [key, value] : headers->get()) {
      auto header = key + ": " + value;
      curl_headers = curl_slist_append(curl_headers, header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
  }

  std::string readBuffer;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

  // Perform the request
  auto res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    return cpp::fail("CURL request failed: " +
                     static_cast<std::string>(curl_easy_strerror(res)));
  }

  curl_easy_cleanup(curl);
  return readBuffer;
}

inline cpp::result<YAML::Node, std::string> ReadRemoteYaml(
    const std::string& url,
    const std::optional<std::reference_wrapper<
        const std::unordered_map<std::string, std::string>>>& headers =
        std::nullopt) {
  auto result = SimpleGet(url, headers);
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

inline cpp::result<Json::Value, std::string> SimpleGetJson(
    const std::string& url,
    const std::optional<std::reference_wrapper<
        const std::unordered_map<std::string, std::string>>>& headers =
        std::nullopt) {

  auto result = SimpleGet(url, headers);
  if (result.has_error()) {
    return cpp::fail(result.error());
  }
  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(result.value(), root)) {
    return cpp::fail("JSON from " + url +
                     " parsing error: " + reader.getFormattedErrorMessages());
  }

  return root;
}
}  // namespace curl_utils
