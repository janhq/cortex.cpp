#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace string_utils {

struct ParsePromptResult {
  std::string user_prompt;
  std::string system_prompt;
  std::string ai_prompt;
};

inline std::string RTrim(const std::string& str) {
  size_t end = str.find_last_not_of("\n\t ");
  return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

inline void Trim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

inline std::string RemoveSubstring(std::string_view full_str,
                                   std::string_view to_remove) {
  if (to_remove.empty()) {
    return std::string(full_str);
  }
  std::string result;
  result.reserve(full_str.length());

  size_t pos = 0;
  size_t prev = 0;

  // Find each occurrence and copy only the parts we want to keep
  while ((pos = full_str.find(to_remove, prev)) != std::string_view::npos) {
    result.append(full_str.substr(prev, pos - prev));
    prev = pos + to_remove.length();
  }

  // Append the remaining part
  result.append(full_str.substr(prev));

  return result;
}

inline bool StringContainsIgnoreCase(const std::string& haystack,
                                     const std::string& needle) {
  if (needle.empty()) {
    return true;
  }

  if (haystack.length() < needle.length()) {
    return false;
  }

  auto it =
      std::search(haystack.begin(), haystack.end(), needle.begin(),
                  needle.end(), [](char ch1, char ch2) {
                    return std::tolower(static_cast<unsigned char>(ch1)) ==
                           std::tolower(static_cast<unsigned char>(ch2));
                  });
  return it != haystack.end();
}

inline bool EqualsIgnoreCase(const std::string& a, const std::string& b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                    [](char a, char b) { return tolower(a) == tolower(b); });
}

inline ParsePromptResult ParsePrompt(const std::string& prompt) {
  auto& pt = prompt;
  ParsePromptResult result;
  result.user_prompt =
      pt.substr(pt.find_first_of('}') + 1,
                pt.find_last_of('{') - pt.find_first_of('}') - 1);
  result.ai_prompt = pt.substr(pt.find_last_of('}') + 1);
  result.system_prompt = pt.substr(0, pt.find_first_of('{'));
  return result;
}
inline bool StartsWith(const std::string& str, const std::string& prefix) {
  return str.rfind(prefix, 0) == 0;
}

inline void SortStrings(std::vector<std::string>& strings) {
  std::sort(strings.begin(), strings.end());
}

inline bool EndsWith(const std::string& str, const std::string& suffix) {
  if (str.length() >= suffix.length()) {
    return (0 == str.compare(str.length() - suffix.length(), suffix.length(),
                             suffix));
  }
  return false;
}

inline std::vector<std::string> SplitBy(const std::string& str,
                                        const std::string&& delimiter) {
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

inline uint64_t getCurrentTimeInMilliseconds() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
      .count();
}

inline std::string FormatTimeElapsed(uint64_t pastTimestamp) {
  uint64_t currentTimestamp = getCurrentTimeInMilliseconds();
  uint64_t milliseconds = currentTimestamp - pastTimestamp;

  // Constants for time units
  const uint64_t millisInSecond = 1000;
  const uint64_t millisInMinute = millisInSecond * 60;
  const uint64_t millisInHour = millisInMinute * 60;
  const uint64_t millisInDay = millisInHour * 24;

  uint64_t days = milliseconds / millisInDay;
  milliseconds %= millisInDay;

  uint64_t hours = milliseconds / millisInHour;
  milliseconds %= millisInHour;

  uint64_t minutes = milliseconds / millisInMinute;
  milliseconds %= millisInMinute;

  uint64_t seconds = milliseconds / millisInSecond;

  std::ostringstream oss;

  if (days > 0) {
    oss << days << " day" << (days > 1 ? "s" : "") << ", ";
  }
  if (hours > 0 || days > 0) {
    oss << hours << " hour" << (hours > 1 ? "s" : "") << ", ";
  }
  if (minutes > 0 || hours > 0 || days > 0) {
    oss << minutes << " minute" << (minutes > 1 ? "s" : "") << ", ";
  }

  oss << seconds << " second" << (seconds > 1 ? "s" : "");

  return oss.str();
}

inline std::string EscapeJson(const std::string& s) {
  std::ostringstream o;
  for (auto c = s.cbegin(); c != s.cend(); c++) {
    switch (*c) {
      case '"':
        o << "\\\"";
        break;
      case '\\':
        o << "\\\\";
        break;
      case '\b':
        o << "\\b";
        break;
      case '\f':
        o << "\\f";
        break;
      case '\n':
        o << "\\n";
        break;
      case '\r':
        o << "\\r";
        break;
      case '\t':
        o << "\\t";
        break;
      default:
        if ('\x00' <= *c && *c <= '\x1f') {
          o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(*c);
        } else {
          o << *c;
        }
    }
  }
  return o.str();
}

// Add a method to compares two url paths
inline bool AreUrlPathsEqual(const std::string& path1,
                             const std::string& path2) {
  auto has_placeholder = [](const std::string& s) {
    if (s.empty())
      return false;
    return s.find_first_of('{') < s.find_last_of('}');
  };
  std::vector<std::string> parts1 = SplitBy(path1, "/");
  std::vector<std::string> parts2 = SplitBy(path2, "/");

  // Check if both strings have the same number of parts
  if (parts1.size() != parts2.size()) {
    return false;
  }

  for (size_t i = 0; i < parts1.size(); ++i) {
    if (has_placeholder(parts1[i]) || has_placeholder(parts2[i]))
      continue;
    if (parts1[i] != parts2[i]) {
      return false;
    }
  }

  return true;
}
}  // namespace string_utils
