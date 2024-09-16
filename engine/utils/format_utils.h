#pragma once

#include <iomanip>
#include <sstream>
#include <string>

namespace format_utils {
inline std::string BytesToHumanReadable(uint64_t bytes) {
  const uint64_t KB = 1024;
  const uint64_t MB = KB * 1024;
  const uint64_t GB = MB * 1024;
  const uint64_t TB = GB * 1024;

  double result;
  std::string unit;

  if (bytes >= TB) {
    result = static_cast<double>(bytes) / TB;
    unit = "TB";
  } else if (bytes >= GB) {
    result = static_cast<double>(bytes) / GB;
    unit = "GB";
  } else if (bytes >= MB) {
    result = static_cast<double>(bytes) / MB;
    unit = "MB";
  } else if (bytes >= KB) {
    result = static_cast<double>(bytes) / KB;
    unit = "KB";
  } else {
    result = static_cast<double>(bytes);
    unit = "B";
  }

  std::ostringstream out;
  // take 2 decimal points
  out << std::fixed << std::setprecision(2) << result << " " << unit;
  return out.str();
}
}  // namespace format_utils
