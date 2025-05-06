#pragma once

#include <json/json.h>
#include <regex>
#include <sstream>
#include <string>
#include "llama3.1.h"

namespace function_calling_utils {
constexpr auto custom_template_function = "<CUSTOM_FUNCTIONS>";

constexpr auto gamma_json = R"(
root   ::= object
value  ::= object | array | string | number | ("true" | "false" | "null") ws

object ::=
  "{" ws (
            string ":" ws value
    ("," ws string ":" ws value)*
  )? "}" ws

array  ::=
  "[" ws (
            value
    ("," ws value)*
  )? "]" ws

string ::=
  "\"" (
    [^"\\\x7F\x00-\x1F] |
    "\\" (["\\bfnrt] | "u" [0-9a-fA-F]{4}) # escapes
  )* "\"" ws

number ::= ("-"? ([0-9] | [1-9] [0-9]{0,15})) ("." [0-9]+)? ([eE] [-+]? [0-9] [1-9]{0,15})? ws

# Optional space: by convention, applied in this grammar after literal chars when allowed
ws ::= | " " | "\n" [ \t]{0,20})";

inline std::string ReplaceCustomFunctions(const std::string& original,
                                          const std::string& replacement) {
  std::string result = original;

  size_t pos = result.find(custom_template_function);
  if (pos != std::string::npos) {
    result.replace(pos, std::string(custom_template_function).length(),
                   replacement);
  }

  return result;
}

inline bool HasTools(const std::shared_ptr<Json::Value>& request) {
  return (request->isMember("tools") && (*request)["tools"].isArray() &&
          (*request)["tools"].size() > 0) ||
         request->get("tools_call_in_user_message", false).asBool();
}

inline std::string ProcessTools(const std::shared_ptr<Json::Value>& request) {
  if (!HasTools(request)) {
    return "";
  }

  std::ostringstream result;
  result << "\n";

  const Json::Value& tools = (*request)["tools"];
  for (const auto& tool : tools) {
    if (tool["type"] == "function") {
      const Json::Value& function = tool["function"];
      result << "Use the function '" << function["name"].asString()
             << "' to: " << function["description"].asString() << "\n";

      Json::FastWriter writer;
      std::string jsonString = writer.write(tool);
      result << jsonString << "\n";
    }
  }

  return result.str();
}

inline Json::Value ParseMultipleFunctionStrings(const std::string& input) {
  Json::Value results(Json::arrayValue);

  // Regular expression to match the function name and arguments
  std::regex functionRegex("<function=([^>]+)>(.+?)</function>");

  // Iterator for regex matches
  auto words_begin =
      std::sregex_iterator(input.begin(), input.end(), functionRegex);
  auto words_end = std::sregex_iterator();

  for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;
    if (match.size() == 3) {
      Json::Value function;
      function["type"] = "function";
      function["function"]["name"] = match[1].str();
      function["function"]["arguments"] = match[2].str();
      results.append(function);
    }
  }

  return results;
}

inline std::string ConvertJsonToFunctionStrings(const Json::Value& jsonArray) {
  if (!jsonArray.isArray()) {
    return "";  // Return empty string if input is not an array
  }

  std::ostringstream result;

  for (const auto& function : jsonArray) {
    auto function_json = function.get("function", {});
    if (function_json.isMember("name") && function_json.isMember("arguments")) {
      result << "<function=" << function_json["name"].asString() << ">"
             << function_json["arguments"].asString() << "</function>";
    }
  }
  return result.str();
}

// Helper function to parse a JSON string to Json
inline Json::Value ParseJsonString(const std::string& jsonString) {
  Json::Value root;
  Json::Reader reader;
  reader.parse(jsonString, root);
  return root;
}

}  // namespace function_calling_utils
