#pragma once

#include <curl/curl.h>
#include <json/reader.h>
#include <json/value.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <optional>
#include <string>
#include <unordered_map>

#include "utils/result.hpp"

enum class RequestType { GET, PATCH, POST, DEL };

namespace curl_utils {

struct Header {
  std::unordered_map<std::string, std::string> m;
};

std::shared_ptr<Header> GetHeaders(const std::string& url);

cpp::result<std::string, std::string> SimpleGet(const std::string& url,
                                                const int timeout = -1);

cpp::result<std::string, std::string> SimpleRequest(
    const std::string& url, const RequestType& request_type,
    const std::string& body = "");

cpp::result<YAML::Node, std::string> ReadRemoteYaml(const std::string& url);

/**
 * SimpleGetJson is a helper function that sends a GET request to the given URL
 *
 * [timeout] is an optional parameter that specifies the timeout for the request. In second.
 */
cpp::result<Json::Value, std::string> SimpleGetJson(const std::string& url,
                                                    const int timeout = -1);
cpp::result<Json::Value, std::string> SimpleGetJsonRecursive(const std::string& url,
                                                    const int timeout = -1);

cpp::result<Json::Value, std::string> SimplePostJson(
    const std::string& url, const std::string& body = "");

cpp::result<Json::Value, std::string> SimpleDeleteJson(
    const std::string& url, const std::string& body = "");

cpp::result<Json::Value, std::string> SimplePatchJson(
    const std::string& url, const std::string& body = "");

}  // namespace curl_utils
