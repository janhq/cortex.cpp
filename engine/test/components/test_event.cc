
#include "common/event.h"
#include "gtest/gtest.h"
#include "utils/json_helper.h"

class EventTest : public ::testing::Test {};

TEST_F(EventTest, EventFromString) {
  // clang-format off
  std::string ev_str = R"({
    "task": {
      "id": "tinyllama:gguf",
      "items": [
        {
          "bytes": 668788096,
          "checksum": "N/A",
          "download_url": "https://huggingface.co/cortexso/tinyllama/resolve/gguf/model.gguf",
          "downloaded_bytes": 0,
          "id": "model.gguf",
          "local_path":
              "/home/jan/cortexcpp/models/cortex.so/tinyllama/gguf/model.gguf"
        },
        {
          "bytes": 545,
          "checksum": "N/A",
          "download_url": "https://huggingface.co/cortexso/tinyllama/resolve/gguf/model.yml",
          "downloaded_bytes": 0,
          "id": "model.yml",
          "local_path":
              "/home/jan/cortexcpp/models/cortex.so/tinyllama/gguf/model.yml"
        }
      ],
      "type": "Model"
    },
    "type": "DownloadStarted"
  })";
  // clang-format on
  auto root = json_helper::ParseJsonString(ev_str);

  auto download_item =
      common::GetDownloadItemFromJson(root["task"]["items"][0]);
  EXPECT_EQ(download_item.download_url,
            root["task"]["items"][0]["download_url"].asString());

  auto download_task = common::GetDownloadTaskFromJson(root["task"]);

  auto ev = cortex::event::GetDownloadEventFromJson(root);
  EXPECT_EQ(ev.type_, cortex::event::DownloadEventType::DownloadStarted);
}
