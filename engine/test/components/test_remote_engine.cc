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