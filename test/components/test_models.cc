#include "gtest/gtest.h"
#include "models/chat_completion_request.h"

using inferences::ChatCompletionRequest;

class ModelTest : public ::testing::Test {
};


TEST_F(ModelTest, should_parse_request) {
  {
    Json::Value data;
    auto req = drogon::HttpRequest::newHttpJsonRequest(data);

    auto res =
        drogon::fromRequest<inferences::ChatCompletionRequest>(*req.get());

    EXPECT_EQ(res.stream, false);
    EXPECT_EQ(res.max_tokens, 500);
    EXPECT_EQ(res.top_p, 0.95f);
    EXPECT_EQ(res.temperature, 0.8f);
    EXPECT_EQ(res.frequency_penalty, 0);
    EXPECT_EQ(res.presence_penalty, 0);
    EXPECT_EQ(res.stop, Json::Value{});
    EXPECT_EQ(res.messages, Json::Value{});
  }

  {
    Json::Value data;
    data["stream"] = true;
    data["max_tokens"] = 400;
    data["top_p"] = 0.8;
    data["temperature"] = 0.7;
    data["frequency_penalty"] = 0.1;
    data["presence_penalty"] = 0.2;
    data["messages"] = "message";
    data["stop"] = "stop";

    auto req = drogon::HttpRequest::newHttpJsonRequest(data);

    auto res =
        drogon::fromRequest<inferences::ChatCompletionRequest>(*req.get());

    EXPECT_EQ(res.stream, true);
    EXPECT_EQ(res.max_tokens, 400);
    EXPECT_EQ(res.top_p, 0.8f);
    EXPECT_EQ(res.temperature, 0.7f);
    EXPECT_EQ(res.frequency_penalty, 0.1f);
    EXPECT_EQ(res.presence_penalty, 0.2f);
    EXPECT_EQ(res.stop, Json::Value{"stop"});
    EXPECT_EQ(res.messages, Json::Value{"message"});
  }
}
