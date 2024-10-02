#pragma once

#include <yaml-cpp/yaml.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
namespace format_utils {

inline std::string writeKeyValue(const std::string& key,
                                 const YAML::Node& value,
                                 const std::string& comment = "") {
  std::ostringstream outFile;
  if (!value)
    return "";
  outFile << key << ": ";

  // Check if the value is a float and round it to 6 decimal places
  if (value.IsScalar()) {
    try {
      double doubleValue = value.as<double>();
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(6) << doubleValue;
      std::string strValue = oss.str();
      // Remove trailing zeros after the decimal point
      strValue.erase(strValue.find_last_not_of('0') + 1, std::string::npos);
      if (strValue.back() == '.') {
        strValue.pop_back();
      }
      outFile << strValue;
    } catch (const std::exception& e) {
      outFile << value;  // If not a float, write as is
    }
  } else {
    outFile << value;
  }

  if (!comment.empty()) {
    outFile << " # " << comment;
  }
  outFile << "\n";
  return outFile.str();
};

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
