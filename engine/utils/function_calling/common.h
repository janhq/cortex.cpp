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

inline std::string CreateCustomFunctionsString(
    std::shared_ptr<Json::Value> request) {
  std::string customFunctions = ProcessTools(request);
  if (customFunctions.empty()) {
    return "";  // No custom functions found
  }

  return "```\n" + customFunctions + "```";
}
inline bool IsValidToolChoiceFormat(const Json::Value& root) {
  return root.isObject() && root.isMember("type") && root["type"].isString() &&
         root["type"].asString() == "function" && root.isMember("function") &&
         root["function"].isObject() && root["function"].isMember("name") &&
         root["function"]["name"].isString();
}
inline void UpdateMessages(std::string& system_prompt,
                           std::shared_ptr<Json::Value> request) {
  Json::Value tool_choice = request->get("tool_choice", "auto");
  if (tool_choice.isString() && tool_choice.asString() == "required") {
    system_prompt +=
        "\n\nYou must call a function to answer the user's question.";
  } else if (!tool_choice.isString()) {

    system_prompt +=
        "\n\nNow this is your first priority: You must call the function '" +
        tool_choice["function"]["name"].asString() +
        "' to answer the user's question.";
  }

  bool tools_call_in_user_message =
      request->get("tools_call_in_user_message", false).asBool();

  bool original_stream_config = (*request).get("stream", false).asBool();
  //   (*request)["grammar"] = function_calling_utils::gamma_json;
  (*request)["stream"] =
      false;  //when using function calling, disable stream automatically because we need to parse the response to get function name and params

  if (!request->isMember("messages") || !(*request)["messages"].isArray() ||
      (*request)["messages"].empty()) {
    // If no messages, add the system prompt as the first message
    Json::Value systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = system_prompt;
    (*request)["messages"].append(systemMessage);
  } else {

    if (tools_call_in_user_message) {
      for (Json::Value& message : (*request)["messages"]) {
        if (message["role"] == "user" && message.isMember("tools") &&
            message["tools"].isArray() && message["tools"].size() > 0) {
          message["content"] = system_prompt + "\n User question: " +
                               message["content"].asString();
        }
      }
    } else {
      Json::Value& firstMessage = (*request)["messages"][0];
      if (firstMessage["role"] == "system") {
        bool addCustomPrompt =
            request->get("add_custom_system_prompt", true).asBool();
        if (addCustomPrompt) {
          firstMessage["content"] =
              system_prompt + "\n" + firstMessage["content"].asString();
        }
      } else {
        // If the first message is not a system message, prepend the system prompt
        Json::Value systemMessage;
        systemMessage["role"] = "system";
        systemMessage["content"] = system_prompt;
        (*request)["messages"].insert(0, systemMessage);
      }
    }

    // transform last message role to tool if it is a function call
    Json::Value& lastMessage =
        (*request)["messages"][(*request)["messages"].size() - 1];
    if (lastMessage.get("role", "") == "tool") {
      lastMessage["role"] = function_calling_llama3_1_utils::tool_role;
      (*request)["stream"] =
          original_stream_config;  // if role is tool then should restore stream config to original value
    }
  }
  for (Json::Value& message : (*request)["messages"]) {
    if (message["role"] == "assistant" && message.isMember("tool_calls")) {
      const Json::Value& tool_calls = message["tool_calls"];
      if (!tool_calls.isNull() && tool_calls.isArray() &&
          tool_calls.size() > 0) {
        message["content"] = ConvertJsonToFunctionStrings(tool_calls);
        message["tool_calls"] = {};
      }
    }
  }
}
inline void PreprocessRequest(std::shared_ptr<Json::Value> request) {
  if (!function_calling_utils::HasTools(request)) {
    return;  // Exit if no tools present
  }
  if (request->get("tool_choice", "auto").isString()) {
    std::string tool_choice = request->get("tool_choice", "auto").asString();
    if (tool_choice == "none") {
      return;  // Exit if tool_choice is none
    }
  }
  std::string customFunctionsString =
      function_calling_utils::CreateCustomFunctionsString(request);
  std::string new_system_prompt =
      function_calling_utils::ReplaceCustomFunctions(
          function_calling_llama3_1_utils::system_prompt,
          customFunctionsString);
  UpdateMessages(new_system_prompt, request);
}

inline void PostProcessResponse(Json::Value& response) {
  if (!response.isMember("choices") || !response["choices"].isArray() ||
      response["choices"].empty()) {
    // If there are no choices or the structure is incorrect, do nothing
    return;
  }

  // Get a reference to the first choice
  Json::Value& firstChoice = response["choices"][0];

  // Check if the choice has a message with content
  if (firstChoice.isMember("message") &&
      firstChoice["message"].isMember("content")) {
    std::string content = firstChoice["message"]["content"].asString();

    // Create a new structure for tool_calls
    Json::Value toolCall = ParseMultipleFunctionStrings(content);
    if (toolCall.size() > 0) {
      // Add tool_calls to the message
      if (response.get("tool_choice", "auto").isString()) {
        std::string tool_choice =
            response.get("tool_choice", "auto").asString();
        if (tool_choice == "auto") {
          firstChoice["finish_reason"] = "tool_calls";
        } else {
          firstChoice["finish_reason"] = "stop";
        }
      }

      firstChoice["message"]["tool_calls"] = toolCall;

      // Clear the content as it's now represented in tool_calls
      firstChoice["message"]["content"] = "";
    }
  }

  // Add any additional post-processing logic here
}
}  // namespace function_calling_utils
