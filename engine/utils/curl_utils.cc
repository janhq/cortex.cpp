#include "curl_utils.h"

#include "utils/engine_constants.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

#include "utils/string_utils.h"
#include "utils/url_parser.h"

namespace curl_utils {
namespace {
class CurlResponse {
 public:
  static size_t WriteCallback(char* buffer, size_t size, size_t nitems,
                              void* userdata) {
    auto* response = static_cast<CurlResponse*>(userdata);
    return response->Append(buffer, size * nitems);
  }

  size_t Append(const char* buffer, size_t size) {
    data_.append(buffer, size);
    return size;
  }

  const std::string& GetData() const { return data_; }

 private:
  std::string data_;
};

void SetUpProxy(CURL* handle, const std::string& url) {
  auto config = file_manager_utils::GetCortexConfig();
  if (!config.proxyUrl.empty()) {
    auto proxy_url = config.proxyUrl;
    auto verify_proxy_ssl = config.verifyProxySsl;
    auto verify_proxy_host_ssl = config.verifyProxyHostSsl;

    auto verify_ssl = config.verifyPeerSsl;
    auto verify_host_ssl = config.verifyHostSsl;

    auto proxy_username = config.proxyUsername;
    auto proxy_password = config.proxyPassword;
    auto no_proxy = config.noProxy;

    CTL_INF("=== Proxy configuration ===");
    CTL_INF("Request url: " << url);
    CTL_INF("Proxy url: " << proxy_url);
    CTL_INF("Verify proxy ssl: " << verify_proxy_ssl);
    CTL_INF("Verify proxy host ssl: " << verify_proxy_host_ssl);
    CTL_INF("Verify ssl: " << verify_ssl);
    CTL_INF("Verify host ssl: " << verify_host_ssl);
    CTL_INF("No proxy: " << no_proxy);

    curl_easy_setopt(handle, CURLOPT_PROXY, proxy_url.c_str());
    if (string_utils::StartsWith(proxy_url, "https")) {
      curl_easy_setopt(handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
    }
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, verify_ssl ? 1L : 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, verify_host_ssl ? 2L : 0L);

    curl_easy_setopt(handle, CURLOPT_PROXY_SSL_VERIFYPEER,
                     verify_proxy_ssl ? 1L : 0L);
    curl_easy_setopt(handle, CURLOPT_PROXY_SSL_VERIFYHOST,
                     verify_proxy_host_ssl ? 2L : 0L);

    auto proxy_auth = proxy_username + ":" + proxy_password;
    curl_easy_setopt(handle, CURLOPT_PROXYUSERPWD, proxy_auth.c_str());

    curl_easy_setopt(handle, CURLOPT_NOPROXY, no_proxy.c_str());
  }
}
}  // namespace

std::shared_ptr<Header> GetHeaders(const std::string& url) {
  auto url_obj = url_parser::FromUrlString(url);
  if (url_obj.has_error()) {
    return nullptr;
  }

  if (url_obj->host == kHuggingFaceHost) {
    auto headers = std::make_shared<Header>();
    headers->m["Content-Type"] = "application/json";
    auto const& token = file_manager_utils::GetCortexConfig().huggingFaceToken;
    if (!token.empty()) {
      headers->m["Authorization"] = "Bearer " + token;

      // for debug purpose
      auto min_token_size = 6;
      if (token.size() < (unsigned)min_token_size) {
        CTL_WRN("Hugging Face token is too short");
      } else {
        CTL_INF("Using authentication with Hugging Face token: " +
                token.substr(token.size() - min_token_size));
      }
    }

    return headers;
  }

  if (url_obj->host == kGitHubHost) {
    auto headers = std::make_shared<Header>();
    headers->m["Accept"] = "application/vnd.github.v3+json";
    // github API requires user-agent https://docs.github.com/en/rest/using-the-rest-api/getting-started-with-the-rest-api?apiVersion=2022-11-28#user-agent
    auto user_agent = file_manager_utils::GetCortexConfig().gitHubUserAgent;
    auto gh_token = file_manager_utils::GetCortexConfig().gitHubToken;
    headers->m["User-Agent"] =
        user_agent.empty() ? kDefaultGHUserAgent : user_agent;
    if (!gh_token.empty()) {
      headers->m["Authorization"] = "Bearer " + gh_token;

      // for debug purpose
      auto min_token_size = 6;
      if (gh_token.size() < (unsigned)min_token_size) {
        CTL_WRN("Github token is too short");
      } else {
        CTL_INF("Using authentication with Github token: " +
                gh_token.substr(gh_token.size() - min_token_size));
      }
    }
    return headers;
  }

  return nullptr;
}

cpp::result<std::string, std::string> SimpleGet(const std::string& url,
                                                const int timeout) {
  auto curl = curl_easy_init();

  if (!curl) {
    return cpp::fail("Failed to init CURL");
  }

  auto headers = GetHeaders(url);
  curl_slist* curl_headers = nullptr;
  if (headers) {
    for (const auto& [key, value] : headers->m) {
      auto header = key + ": " + value;
      curl_headers = curl_slist_append(curl_headers, header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
  }

  auto* response = new CurlResponse();
  std::shared_ptr<CurlResponse> s(response,
                                  std::default_delete<CurlResponse>());

  SetUpProxy(curl, url);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
  if (timeout > 0) {
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  }

  // Perform the request
  auto res = curl_easy_perform(curl);

  curl_slist_free_all(curl_headers);
  curl_easy_cleanup(curl);
  if (res != CURLE_OK) {
    return cpp::fail("CURL request failed: " +
                     static_cast<std::string>(curl_easy_strerror(res)));
  }
  auto http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (http_code >= 400) {
    CTL_ERR("HTTP request failed with status code: " +
            std::to_string(http_code));
    return cpp::fail(response->GetData());
  }

  return response->GetData();
}

cpp::result<std::string, std::string> SimpleRequest(
    const std::string& url, const RequestType& request_type,
    const std::string& body) {
  auto curl = curl_easy_init();

  if (!curl) {
    return cpp::fail("Failed to init CURL");
  }

  auto headers = GetHeaders(url);
  curl_slist* curl_headers = nullptr;
  curl_headers =
      curl_slist_append(curl_headers, "Content-Type: application/json");
  curl_headers = curl_slist_append(curl_headers, "Expect:");

  if (headers) {
    for (const auto& [key, value] : headers->m) {
      auto header = key + ": " + value;
      curl_headers = curl_slist_append(curl_headers, header.c_str());
    }
  }
  auto* response = new CurlResponse();
  std::shared_ptr<CurlResponse> s(response,
                                  std::default_delete<CurlResponse>());

  SetUpProxy(curl, url);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  if (request_type == RequestType::PATCH) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
  } else if (request_type == RequestType::POST) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
  } else if (request_type == RequestType::DEL) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  }
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlResponse::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.length());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

  // Perform the request
  auto res = curl_easy_perform(curl);

  auto http_code = 0L;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  // Clean up
  curl_slist_free_all(curl_headers);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    CTL_ERR("CURL request failed: " + std::string(curl_easy_strerror(res)));
    return cpp::fail("CURL request failed: " +
                     static_cast<std::string>(curl_easy_strerror(res)));
  }

  if (http_code >= 400) {
    CTL_ERR("HTTP request failed with status code: " +
            std::to_string(http_code));
    return cpp::fail(response->GetData());
  }

  return response->GetData();
}

cpp::result<YAML::Node, std::string> ReadRemoteYaml(const std::string& url) {
  auto result = SimpleGet(url);
  if (result.has_error()) {
    CTL_ERR("Failed to get Yaml from " + url + ": " + result.error());
    return cpp::fail(result.error());
  }

  try {
    return YAML::Load(result.value());
  } catch (const std::exception& e) {
    return cpp::fail("YAML from " + url +
                     " parsing error: " + std::string(e.what()));
  }
}

cpp::result<Json::Value, std::string> SimpleGetJson(const std::string& url,
                                                    const int timeout) {
  auto result = SimpleGet(url, timeout);
  if (result.has_error()) {
    CTL_ERR("Failed to get JSON from " + url + ": " + result.error());
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

cpp::result<Json::Value, std::string> SimpleGetJsonRecursive(
    const std::string& url, const int timeout) {
  auto result = SimpleGetJson(url, timeout);
  if (result.has_error()) {
    return result;
  }
  auto root = result.value();

  if (root.isArray()) {
    for (const auto& value : root) {
      if (value["type"].asString() == "directory") {
        auto temp = SimpleGetJsonRecursive(
            url + "/" +
                std::filesystem::path(value["path"].asString())
                    .filename()
                    .string(),
            timeout);
        if (!temp.has_error()) {
          if (temp.value().isArray()) {
            for (const auto& item : temp.value()) {
              root.append(item);
            }
          } else {
            root.append(temp.value());
          }
        }
      }
    }
    for (Json::ArrayIndex i = 0; i < root.size();) {
      if (root[i].isMember("type") && root[i]["type"] == "directory") {
        root.removeIndex(i, nullptr);
      } else {
        ++i;
      }
    }
  }
  return root;
}

cpp::result<Json::Value, std::string> SimplePostJson(const std::string& url,
                                                     const std::string& body) {
  auto result = SimpleRequest(url, RequestType::POST, body);
  if (result.has_error()) {
    CTL_INF("url: " + url);
    CTL_INF("body: " + body);
    CTL_ERR("Failed to get JSON from " + url + ": " + result.error());
    return cpp::fail(result.error());
  }

  CTL_INF("Response: " + result.value());
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(result.value(), root)) {
    return cpp::fail("JSON from " + url +
                     " parsing error: " + reader.getFormattedErrorMessages());
  }

  return root;
}

cpp::result<Json::Value, std::string> SimpleDeleteJson(
    const std::string& url, const std::string& body) {
  auto result = SimpleRequest(url, RequestType::DEL, body);
  if (result.has_error()) {
    CTL_ERR("Failed to get JSON from " + url + ": " + result.error());
    return cpp::fail(result.error());
  }

  CTL_INF("Response: " + result.value());
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(result.value(), root)) {
    return cpp::fail("JSON from " + url +
                     " parsing error: " + reader.getFormattedErrorMessages());
  }

  return root;
}

cpp::result<Json::Value, std::string> SimplePatchJson(const std::string& url,
                                                      const std::string& body) {
  auto result = SimpleRequest(url, RequestType::PATCH, body);
  if (result.has_error()) {
    CTL_ERR("Failed to get JSON from " + url + ": " + result.error());
    return cpp::fail(result.error());
  }

  CTL_INF("Response: " + result.value());
  Json::Value root;
  Json::Reader reader;
  if (!reader.parse(result.value(), root)) {
    return cpp::fail("JSON from " + url +
                     " parsing error: " + reader.getFormattedErrorMessages());
  }

  return root;
}
}  // namespace curl_utils
