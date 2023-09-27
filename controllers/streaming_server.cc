#include "streaming_server.h"
#include <random>
#include <string>
#include <thread>

class TextCompletions {
private:
  std::string url;
  int prev_pos = 10;
  int prev_size = 0;

public:
  std::mutex mutex_;
  std::condition_variable cv_;
  std::shared_ptr<std::vector<std::string>> result_list =
      std::make_shared<std::vector<std::string>>();
  TextCompletions(std::string llm_host) {
    this->url = llm_host;
  }
  void GetCompletions(std::string model_name, std::string text_prompt,
                      int max_tokens, float top_p, float temperature,
                      uint64_t sequence_id, bool sequence_start,
                      int stream_timeout) {
    for (int i = 0; i < 10; i++){
      result_list->push_back("test");
    }
    // result_list->push_back("A [DONE]");
    return;
  }
};

std::string generate_random_string(std::size_t length) {
  const std::string characters =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  std::random_device rd;
  std::mt19937 generator(rd());

  std::uniform_int_distribution<> distribution(0, characters.size() - 1);

  std::string random_string(length, '\0');
  std::generate_n(random_string.begin(), length,
                  [&]() { return characters[distribution(generator)]; });

  return random_string;
}

std::string create_return_json(const std::string &id, const std::string &model,
                               const std::string &content,
                               Json::Value finish_reason = Json::Value()) {

  Json::Value root;

  root["id"] = id;
  root["model"] = model;
  root["created"] = static_cast<int>(std::time(nullptr));
  root["object"] = "chat.completion.chunk";

  Json::Value choicesArray(Json::arrayValue);
  Json::Value choice;

  choice["index"] = 0;
  Json::Value delta;
  delta["content"] = content;
  choice["delta"] = delta;
  choice["finish_reason"] = finish_reason;

  choicesArray.append(choice);
  root["choices"] = choicesArray;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // This sets the indentation to an empty string,
                              // producing compact output.
  return Json::writeString(writer, root);
}

void llm_models::chatCompletion(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback) {
  drogon::HttpResponsePtr resp;

  const auto &jsonBody = req->getJsonObject();

  std::string formatted_output =
      "Below is a conversation between an AI system named ASSISTANT and USER\n";

  auto app_config = drogon::app().getCustomConfig();

  std::string url = app_config["triton_endpoint"].asString();

  int sequence_id = 1000;
  this->ids_count++;
  LOG_INFO << "Starting ids_count:" << this->ids_count;
  bool sequence_start = true;
  bool sequence_end = false;
  std::string model_name = "turbomind";
  // This will be input
  int max_tokens = 1000;
  float top_p = 0.8;
  float temperature = 0.8;
  int is_start = sequence_start ? 1 : 0;
  int is_end = sequence_end ? 1 : 0;
  uint64_t correlated_id = 1;
  uint64_t random_seed = 2929876282353812494;
  std::shared_ptr<TextCompletions> textCompletionsPtr =
      std::make_shared<TextCompletions>(url);

  if (jsonBody) {
    max_tokens = (*jsonBody)["max_tokens"].asInt();
    top_p = (*jsonBody)["top_p"].asFloat();
    temperature = (*jsonBody)["temperature"].asFloat();

    const Json::Value &messages = (*jsonBody)["messages"];
    for (const auto &message : messages) {
      std::string role = message["role"].asString();
      std::string content = message["content"].asString();
      formatted_output += role + ": " + content + "\n";
    }
  }

  textCompletionsPtr->GetCompletions(model_name, formatted_output, max_tokens,
                                     top_p, temperature, sequence_id, true, 30);

  resp = drogon::HttpResponse::newStreamResponse(
      // Input a lambda as callback
      [this, textCompletionsPtr, url,
       sequence_id](char *pBuffer, std::size_t nBuffSize) -> std::size_t {
        if (!pBuffer) {
          LOG_INFO << "Connection closed or buffer is null.";
          return 0;
        }

        std::string content = "";

        {
          std::unique_lock<std::mutex> lk(textCompletionsPtr->mutex_);
          // Wait for up to 2000ms (2 seconds) for result_list to have data
          bool hasData = textCompletionsPtr->cv_.wait_for(
              lk, std::chrono::milliseconds(2000),
              [&]() { return !textCompletionsPtr->result_list->empty(); });

          if (!hasData) {
            return 0;
          }

          for (const auto &cur_result : *textCompletionsPtr->result_list) {
            LOG_INFO << cur_result;
            if (cur_result.find("[DONE]") != std::string::npos) {
              this->ids_count--;
              LOG_INFO << "Current id_counts: " << this->ids_count;
              return 0; // Return 0 if there's no data after waiting for 500ms
            };
            if (cur_result.find("|[PRE]") != std::string::npos) {
              content += "data: " +
                         create_return_json(
                             generate_random_string(20), "_",
                             "", true) +
                         "\n\n" + "data: [DONE]" + "\n\n";
            } else {
              content += "data: " +
                         create_return_json(
                             generate_random_string(20), "_",
                             cur_result, false) +
                         "\n\n";
            }
            LOG_INFO << content.data();
          }
          textCompletionsPtr->result_list->clear();
        }

        std::size_t nRead = std::min(content.size(), nBuffSize);
        memcpy(pBuffer, content.data(), nRead);
        return nRead;
      }, // end of lambda expression
      "chat_completion.txt");
  resp->setContentTypeString("text/event-stream");

  callback(resp);
  return;
}
