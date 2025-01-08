#include <regex>
#include <string>
#include <unordered_map>
#include "extensions/remote-engine/helper.h"
#include "extensions/template_renderer.h"
#include "gtest/gtest.h"
#include "utils/json_helper.h"

class RemoteEngineTest : public ::testing::Test {};

TEST_F(RemoteEngineTest, OpenAiToAnthropicRequest) {
  std::string tpl =
      R"({ 
      {% for key, value in input_request %}
        {% if key == "messages" %}  
          {% if input_request.messages.0.role == "system" %}
            "system": "{{ input_request.messages.0.content }}",
            "messages": [
              {% for message in input_request.messages %}
                {% if not loop.is_first %}
                  {"role": "{{ message.role }}", "content": "{{ message.content }}" } {% if not loop.is_last %},{% endif %}
                {% endif %}
              {% endfor %}
            ]
          {% else %}
            "messages": [
              {% for message in input_request.messages %}
                {"role": " {{ message.role}}", "content": "{{ message.content }}" } {% if not loop.is_last %},{% endif %}
              {% endfor %}
            ]
          {% endif %}
          {% if not loop.is_last %},{% endif %} 
        {% else if key == "system" or key == "model" or key == "temperature" or key == "store" or key == "max_tokens" or key == "stream" or key == "presence_penalty" or key == "metadata" or key == "frequency_penalty" or key == "tools" or key == "tool_choice" or key == "logprobs" or key == "top_logprobs" or key == "logit_bias" or key == "n" or key == "modalities" or key == "prediction" or key == "response_format" or key == "service_tier" or key == "seed" or key == "stop" or key == "stream_options" or key == "top_p" or key == "parallel_tool_calls" or key == "user" %}
          "{{ key }}": {{ tojson(value) }}   
          {% if not loop.is_last %},{% endif %} 
        {% endif %}      
      {% endfor %} })";
  {
    std::string message_with_system = R"({
    "engine" : "anthropic",
	  "max_tokens" : 1024,
    "messages": [        
        {"role": "system", "content": "You are a seasoned data scientist at a Fortune 500 company."},
        {"role": "user", "content": "Hello, world"}
    ],
    "model": "claude-3-5-sonnet-20241022",
    "stream" : true
})";

    auto data = json_helper::ParseJsonString(message_with_system);

    extensions::TemplateRenderer rdr;
    auto res = rdr.Render(tpl, data);

    auto res_json = json_helper::ParseJsonString(res);
    EXPECT_EQ(data["model"].asString(), res_json["model"].asString());
    EXPECT_EQ(data["max_tokens"].asInt(), res_json["max_tokens"].asInt());
    for (auto const& msg : data["messages"]) {
      if (msg["role"].asString() == "system") {
        EXPECT_EQ(msg["content"].asString(), res_json["system"].asString());
      } else if (msg["role"].asString() == "user") {
        EXPECT_EQ(msg["content"].asString(),
                  res_json["messages"][0]["content"].asString());
      }
    }
  }

  {
    std::string message_without_system = R"({
    "messages": [
        {"role": "user", "content": "Hello, world"}
    ],
    "model": "claude-3-5-sonnet-20241022",
    "max_tokens": 1024,
})";

    auto data = json_helper::ParseJsonString(message_without_system);

    extensions::TemplateRenderer rdr;
    auto res = rdr.Render(tpl, data);

    auto res_json = json_helper::ParseJsonString(res);
    EXPECT_EQ(data["model"].asString(), res_json["model"].asString());
    EXPECT_EQ(data["max_tokens"].asInt(), res_json["max_tokens"].asInt());
    EXPECT_EQ(data["messages"][0]["content"].asString(),
              res_json["messages"][0]["content"].asString());
  }
}

TEST_F(RemoteEngineTest, OpenAiResponse) {
  std::string tpl = R"({ 
    {% set first = true %} 
    {% for key, value in input_request %} 
      {% if  key == "choices" or key == "created" or key == "model" or key == "service_tier" or key == "system_fingerprint" or key == "stream" or key == "object" or key == "usage" %} 
      {% if not first %},{% endif %} 
        "{{ key }}": {{ tojson(value) }} 
      {% set first = false %} 
      {% endif %} 
    {% endfor %} 
  })";
  std::string message = R"(
 {
    "choices": [
      {
        "delta": {
          "content": " questions"
        },
        "finish_reason": null,
        "index": 0
      }
    ],
    "created": 1735372587,
    "id": "",
    "model": "o1-preview",
    "object": "chat.completion.chunk",
    "stream": true,
    "system_fingerprint": "fp_1ddf0263de"
  })";
  auto data = json_helper::ParseJsonString(message);

  extensions::TemplateRenderer rdr;
  auto res = rdr.Render(tpl, data);

  auto res_json = json_helper::ParseJsonString(res);
  EXPECT_EQ(data["model"].asString(), res_json["model"].asString());
  EXPECT_EQ(data["created"].asInt(), res_json["created"].asInt());
  EXPECT_EQ(data["choices"][0]["delta"]["content"].asString(),
            res_json["choices"][0]["delta"]["content"].asString());
}

TEST_F(RemoteEngineTest, AnthropicResponse) {
  std::string tpl = R"(
  {% if input_request.stream %} 
  {"object": "chat.completion.chunk", "model": "{{ input_request.model }}", "choices": [{"index": 0, "delta": { {% if input_request.type == "message_start" %} "role": "assistant", "content": null {% else if input_request.type == "ping" %} "role": "assistant", "content": null {% else if input_request.type == "content_block_delta" %} "role": "assistant", "content": "{{ input_request.delta.text }}" {% else if input_request.type == "content_block_stop" %} "role": "assistant", "content": null {% else if input_request.type == "content_block_stop" %} "role": "assistant", "content": null {% endif %} }, {% if input_request.type == "content_block_stop" %} "finish_reason": "stop" {% else %} "finish_reason": null {% endif %} }]} 
  {% else %} 
    {"id": "{{ input_request.id }}", 
    "created": null, 
    "object": "chat.completion", 
    "model": "{{ input_request.model }}", 
    "choices": [{ 
      "index": 0, 
      "message": { 
        "role": "{{ input_request.role }}", 
        "content": {% if not input_request.content %} null {% else if input_request.content and input_request.content.0.type == "text" %}  {{input_request.content.0.text}} {% else %} null {% endif %}, 
        "refusal": null }, 
      "logprobs": null, 
      "finish_reason": "{{ input_request.stop_reason }}" } ], 
    "usage": { 
      "prompt_tokens": {{ input_request.usage.input_tokens }}, 
      "completion_tokens": {{ input_request.usage.output_tokens }}, 
      "total_tokens": {{ input_request.usage.input_tokens + input_request.usage.output_tokens }}, 
      "prompt_tokens_details": { "cached_tokens": 0 }, 
      "completion_tokens_details": { "reasoning_tokens": 0, "accepted_prediction_tokens": 0, "rejected_prediction_tokens": 0 } }, 
      "system_fingerprint": "fp_6b68a8204b"} 
  {% endif %})";
  std::string message = R"(
 {
    "content": [],
    "id": "msg_01SckpnDyChcmmawQsWHr8CH",
    "model": "claude-3-opus-20240229",
    "role": "assistant",
    "stop_reason": "end_turn",
    "stop_sequence": null,
    "stream": false,
    "type": "message",
    "usage": {
      "cache_creation_input_tokens": 0,
      "cache_read_input_tokens": 0,
      "input_tokens": 130,
      "output_tokens": 3
    }
  })";
  auto data = json_helper::ParseJsonString(message);

  extensions::TemplateRenderer rdr;
  auto res = rdr.Render(tpl, data);

  auto res_json = json_helper::ParseJsonString(res);
  EXPECT_EQ(data["model"].asString(), res_json["model"].asString());
  EXPECT_EQ(data["created"].asInt(), res_json["created"].asInt());
  EXPECT_TRUE(res_json["choices"][0]["message"]["content"].isNull());
}

TEST_F(RemoteEngineTest, HeaderTemplate) {
  {
    std::string header_template =
        R"(x-api-key: {{api_key}} anthropic-version: {{version}})";
    Json::Value test_value;
    test_value["api_key"] = "test";
    test_value["version"] = "test_version";
    std::unordered_map<std::string, std::string> replacements;
    auto r = remote_engine::GetReplacements(header_template);
    for (auto s : r) {
      if (test_value.isMember(s)) {
        replacements.insert({s, test_value[s].asString()});
      }
    }

    auto result =
        remote_engine::ReplaceHeaderPlaceholders(header_template, replacements);

    EXPECT_EQ(result[0], "x-api-key: test");
    EXPECT_EQ(result[1], "anthropic-version: test_version");
  }

  {
    std::string header_template =
        R"(x-api-key: {{api_key}} anthropic-version: test_version)";
    Json::Value test_value;
    test_value["api_key"] = "test";
    test_value["version"] = "test_version";
    std::unordered_map<std::string, std::string> replacements;
    auto r = remote_engine::GetReplacements(header_template);
    for (auto s : r) {
      if (test_value.isMember(s)) {
        replacements.insert({s, test_value[s].asString()});
      }
    }

    auto result =
        remote_engine::ReplaceHeaderPlaceholders(header_template, replacements);

    EXPECT_EQ(result[0], "x-api-key: test");
    EXPECT_EQ(result[1], "anthropic-version: test_version");
  }

  {
    std::string header_template = R"(Authorization: Bearer {{api_key}}")";
    Json::Value test_value;
    test_value["api_key"] = "test";
    std::unordered_map<std::string, std::string> replacements;
    auto r = remote_engine::GetReplacements(header_template);
    for (auto s : r) {
      if (test_value.isMember(s)) {
        replacements.insert({s, test_value[s].asString()});
      }
    }

    auto result =
        remote_engine::ReplaceHeaderPlaceholders(header_template, replacements);

    EXPECT_EQ(result[0], "Authorization: Bearer test");
  }
}