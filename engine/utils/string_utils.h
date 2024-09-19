#include <string>
#include <vector>

namespace string_utils {
inline bool StartsWith(const std::string& str, const std::string& prefix) {
  return str.rfind(prefix, 0) == 0;
}

inline bool EndsWith(const std::string& str, const std::string& suffix) {
  if (str.length() >= suffix.length()) {
    return (0 == str.compare(str.length() - suffix.length(), suffix.length(),
                             suffix));
  }
  return false;
}

inline std::vector<std::string> SplitBy(const std::string& str,
                                        const std::string& delimiter) {
  std::vector<std::string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delimiter, prev);
    if (pos == std::string::npos)
      pos = str.length();
    std::string token = str.substr(prev, pos - prev);
    if (!token.empty())
      tokens.push_back(token);
    prev = pos + delimiter.length();
  } while (pos < str.length() && prev < str.length());
  return tokens;
}
}  // namespace string_utils
