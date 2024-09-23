#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "exceptions/malformed_url_exception.h"

namespace url_parser {
struct Url {
  std::string protocol;
  std::string host;
  std::vector<std::string> pathParams;
  std::unordered_map<std::string, std::variant<std::string, int, bool>> queries;

  std::string GetProtocolAndHost() const { return protocol + "://" + host; }

  std::string GetPathAndQuery() const {
    std::string path;
    for (const auto& path_param : pathParams) {
      path += "/" + path_param;
    }
    std::string query;
    for (const auto& [key, value] : queries) {
      query += key + "=" + std::get<std::string>(value) + "&";
    }
    if (!query.empty()) {
      query.pop_back();
      return path + "?" + query;
    }
    return path;
  };

  std::string ToFullPath() const {
    return GetProtocolAndHost() + GetPathAndQuery();
  }
};

const std::regex url_regex(
    R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
    std::regex::extended);

inline void SplitPathParams(const std::string& input,
                            std::vector<std::string>& pathList) {
  // split the path by '/'
  std::string token;
  std::istringstream tokenStream(input);
  while (std::getline(tokenStream, token, '/')) {
    if (token.empty()) {
      continue;
    }
    pathList.push_back(token);
  }
}

inline Url FromUrlString(const std::string& urlString) {
  Url url = {
      .protocol = "",
      .host = "",
      .pathParams = {},
  };
  unsigned counter = 0;

  std::smatch url_match_result;

  auto protocolIndex{2};
  auto hostAndPortIndex{4};
  auto pathIndex{5};
  auto queryIndex{7};

  if (std::regex_match(urlString, url_match_result, url_regex)) {
    for (const auto& res : url_match_result) {
      if (counter == protocolIndex) {
        url.protocol = res;
      } else if (counter == hostAndPortIndex) {
        url.host = res;  // TODO: split the port for completeness
      } else if (counter == pathIndex) {
        SplitPathParams(res, url.pathParams);
      } else if (counter == queryIndex) {
        // TODO: implement
      }
      counter++;
    }
  } else {
    auto message{"Malformed URL: " + urlString};
    throw MalformedUrlException(message);
  }
  return url;
}

inline std::string FromUrl(const Url& url) {
  if (url.protocol.empty() || url.host.empty()) {
    auto message{"Url must have protocol and host"};
    throw MalformedUrlException(message);
  }
  std::ostringstream url_string;
  url_string << url.protocol << "://" << url.host;

  for (const auto& path : url.pathParams) {
    url_string << "/" << path;
  }

  std::string query_string;
  for (const auto& [key, value] : url.queries) {
    try {
      std::string value_str;
      if (std::holds_alternative<std::string>(value)) {
        value_str = std::get<std::string>(value);
      } else if (std::holds_alternative<int>(value)) {
        value_str = std::to_string(std::get<int>(value));
      } else if (std::holds_alternative<bool>(value)) {
        value_str = std::get<bool>(value) ? "true" : "false";
      }
      if (!query_string.empty()) {
        query_string += "&";
      }
      query_string += key + "=" + value_str;
    } catch (const std::bad_variant_access& e) {
      // Handle the case where the variant does not match any of the expected types
      // This should not happen if the map was created correctly
      throw std::runtime_error("Invalid variant type in queries map");
    }
  }

  if (!query_string.empty()) {
    url_string << "?" << query_string;
  }

  return url_string.str();
}
}  // namespace url_parser
