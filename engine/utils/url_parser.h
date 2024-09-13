#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "exceptions/malformed_url_exception.h"

namespace url_parser {
// TODO: add an unordered map to store the query
// TODO: add a function to construct a string from Url

struct Url {
  std::string protocol;
  std::string host;
  std::vector<std::string> pathParams;
  std::unordered_map<std::string, std::variant<std::string, int, bool>> queries;
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

  // TODO: handle queries

  return url_string.str();
}
}  // namespace url_parser
