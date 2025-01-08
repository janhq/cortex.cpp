#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>

namespace remote_engine {
std::vector<std::string> GetReplacements(const std::string& header_template) {
  std::vector<std::string> replacements;
  std::regex placeholder_regex(R"(\{\{(.*?)\}\})");
  std::smatch match;

  std::string template_copy = header_template;
  while (std::regex_search(template_copy, match, placeholder_regex)) {
    std::string key = match[1].str();
    replacements.push_back(key);
    template_copy = match.suffix().str();
  }

  return replacements;
}

std::vector<std::string> ReplaceHeaderPlaceholders(
    const std::string& header_template,
    const std::unordered_map<std::string, std::string>& replacements) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = header_template.find("}}");

  while (end != std::string::npos) {
    // Extract the part
    std::string part = header_template.substr(start, end - start + 2);

    // Replace variables in this part
    for (const auto& var : replacements) {
      std::string placeholder = "{{" + var.first + "}}";
      size_t pos = part.find(placeholder);
      if (pos != std::string::npos) {
        part.replace(pos, placeholder.length(), var.second);
      }
    }

    // Trim whitespace
    part.erase(0, part.find_first_not_of(" \t\n\r\f\v"));
    part.erase(part.find_last_not_of(" \t\n\r\f\v") + 1);

    // Add to result if not empty
    if (!part.empty()) {
      result.push_back(part);
    }

    // Move to next part
    start = end + 2;
    end = header_template.find("}}", start);
  }

  // Process any remaining part
  if (start < header_template.length()) {
    std::string part = header_template.substr(start);

    // Replace variables in this part
    for (const auto& var : replacements) {
      std::string placeholder = "{{" + var.first + "}}";
      size_t pos = part.find(placeholder);
      if (pos != std::string::npos) {
        part.replace(pos, placeholder.length(), var.second);
      }
    }

    // Trim whitespace
    part.erase(0, part.find_first_not_of(" \t\n\r\f\v"));
    part.erase(part.find_last_not_of(" \t\n\r\f\v") + 1);

    if (!part.empty()) {
      result.push_back(part);
    }
  }
  return result;
}
}  // namespace remote_engine