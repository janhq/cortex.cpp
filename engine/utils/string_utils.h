#pragma once

#include <chrono>
#include <sstream>
#include <string>
#include <vector>

namespace string_utils {

struct ParsePromptResult {
  std::string user_prompt;
  std::string system_prompt;
  std::string ai_prompt;
};

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
}  // namespace string_utils
