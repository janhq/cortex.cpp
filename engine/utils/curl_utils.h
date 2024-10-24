#pragma once

#include <curl/curl.h>
#include <json/reader.h>
#include <json/value.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <string>
#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"
#include "utils/result.hpp"
#include "utils/url_parser.h"

namespace curl_utils {
namespace {
size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                     std::string* output) {
  size_t totalSize = size * nmemb;
  output->append((char*)contents, totalSize);
  return totalSize;
}
}  // namespace

inline std::optional<std::unordered_map<std::string, std::string>> GetHeaders(
    const std::string& url);

inline cpp::result<std::string, std::string> SimpleGet(const std::string& url) {
  // Initialize libcurl
  curl_global_init(CURL_GLOBAL_DEFAULT);
  auto curl = curl_easy_init();

  if (!curl) {
    return cpp::fail("Failed to init CURL");
  }

  auto headers = GetHeaders(url);
  if (headers.has_value()) {
    curl_slist* curl_headers = nullptr;

    for (const auto& [key, value] : headers.value()) {
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

inline cpp::result<Json::Value, std::string> SimpleGetJson(
    const std::string& url) {
  auto result = SimpleGet(url);
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

inline std::optional<std::unordered_map<std::string, std::string>> GetHeaders(
    const std::string& url) {
  auto url_obj = url_parser::FromUrlString(url);
  if (url_obj.has_error()) {
    return std::nullopt;
  }

  if (url_obj->host == kHuggingFaceHost) {
    std::unordered_map<std::string, std::string> headers{};
    headers["Content-Type"] = "application/json";
    auto const& token = file_manager_utils::GetCortexConfig().huggingFaceToken;
    if (!token.empty()) {
      headers["Authorization"] = "Bearer " + token;

      // for debug purpose
      auto min_token_size = 6;
      if (token.size() < min_token_size) {
        CTL_WRN("Hugging Face token is too short");
      } else {
        CTL_INF("Using authentication with Hugging Face token: " +
                token.substr(token.size() - min_token_size));
      }
    }

    return headers;
  }

  if (url_obj->host == kGitHubHost) {
    std::unordered_map<std::string, std::string> headers{};
    headers["Accept"] = "application/vnd.github.v3+json";
    // github API requires user-agent https://docs.github.com/en/rest/using-the-rest-api/getting-started-with-the-rest-api?apiVersion=2022-11-28#user-agent
    auto user_agent = file_manager_utils::GetCortexConfig().gitHubUserAgent;
    headers["User-Agent"] =
        user_agent.empty() ? kDefaultGHUserAgent : user_agent;
    return headers;
  }

  return std::nullopt;
}
}  // namespace curl_utils
