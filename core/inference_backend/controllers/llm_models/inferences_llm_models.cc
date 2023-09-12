#include "inferences_llm_models.h"
#include "inference_lib.h"
#include <random>
#include <string>
#include <thread> // for sleep_for

using namespace inferences;

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

void llm_models::chatCompletion(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  drogon::HttpResponsePtr resp;

  const auto &jsonBody = req->getJsonObject();

  std::string formatted_output =
      "Below is a conversation between an AI system named ASSISTANT and USER\n";

  auto app_config = drogon::app().getCustomConfig();

  std::string url = app_config["triton_endpoint"].asString();

  int sequence_id = inference_utils::generate_random_positive_int(10000);
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
  std::shared_ptr<inference::TextCompletions> textCompletionsPtr =
      std::make_shared<inference::TextCompletions>(url);

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
      [this, textCompletionsPtr, url,
       sequence_id](char *pBuffer, std::size_t nBuffSize) -> std::size_t {
        if (!pBuffer) {
          LOG_INFO << "Connection closed or buffer is null.";
          return 0;
        }

        std::string content = "";

        {
          std::unique_lock<std::mutex> lk(textCompletionsPtr->mutex_);

          // Wait for up to 2000ms (2 seconds)for result_list to have data
          bool hasData = textCompletionsPtr->cv_.wait_for(
              lk, std::chrono::milliseconds(2000),
              [&]() { return !textCompletionsPtr->result_list->empty(); });

          if (!hasData) {
            return 0;
          }

          for (const auto &cur_result : *textCompletionsPtr->result_list) {
            if (cur_result.find("[DONE]") != std::string::npos) {
              this->ids_count--;
              trantor::EventLoop *loop = app().getLoop();
              // TO DO: The inference logic here is not good, need improvement
              loop->runAfter(5, [textCompletionsPtr, url, sequence_id] {
                textCompletionsPtr->triton_client->StopStream();
                textCompletionsPtr->triton_client.reset();
                auto textCompletionsPtrSTOP =
                    std::make_shared<inference::TextCompletions>(url);
                textCompletionsPtrSTOP->GetCompletions("turbomind", "", 0, 1, 1,
                                                       sequence_id, false, 1);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                textCompletionsPtrSTOP->triton_client->StopStream();
                textCompletionsPtrSTOP->triton_client.reset();
              });
              LOG_INFO << "Current id_counts: " << this->ids_count;

              return 0; // Return 0 if there's no data after waiting for 500ms
            };
            if (cur_result.find("|[PRE]") != std::string::npos) {
              content += "data: " +
                         create_return_json(
                             inference_utils::generate_random_string(20), "_",
                             "", true) +
                         "\n\n" + "data: [DONE]" + "\n\n";
            } else {
              content += "data: " +
                         create_return_json(
                             inference_utils::generate_random_string(20), "_",
                             cur_result, false) +
                         "\n\n";
            }
          }
          textCompletionsPtr->result_list->clear();
        }

        std::size_t nRead = std::min(content.size(), nBuffSize);
        memcpy(pBuffer, content.data(), nRead);
        return nRead;
      },
      "chat_completion.txt");
  resp->setContentTypeString("text/event-stream");

  callback(resp);
  return;
}
