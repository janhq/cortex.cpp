#pragma once
#include <chrono>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
namespace string_utils {
constexpr char RESET[] = "\033[0m";
constexpr char BOLD[] = "\033[1m";
constexpr char GREEN[] = "\033[1;32m";
constexpr char YELLOW[] = "\033[0;33m";
constexpr char BLUE[] = "\033[0;34m";
constexpr char MAGENTA[] = "\033[0;35m";
constexpr char GRAY[] = "\033[1;90m";

inline std::string print_comment(const std::string& comment) {
  std::ostringstream oss;
  oss << GRAY << "# " << comment << RESET << "\n";
  return oss.str();
};

inline std::string print_kv(const std::string& key, const std::string& value,
                            const std::string& color = "\033[0m") {
  std::ostringstream oss;
  oss << GREEN << key << ":" << RESET << " " << color << value << RESET << "\n";
  return oss.str();
};

inline std::string print_bool(const std::string& key, bool value) {
  return print_kv(key, value ? "true" : "false", MAGENTA);
};

inline std::string print_float(const std::string& key, float value) {
  if (!std::isnan(value)) {
    std::ostringstream float_oss;
    float_oss << std::fixed << std::setprecision(6) << value;
    std::string str_value = float_oss.str();
    // Remove trailing zeros
    str_value.erase(str_value.find_last_not_of('0') + 1, std::string::npos);
    // Remove trailing dot if present
    if (str_value.back() == '.') {
      str_value.pop_back();
    }
    return print_kv(key, str_value, BLUE);
  } else
    return "";
};
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
