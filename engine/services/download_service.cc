#include <httplib.h>
#include <trantor/utils/Logger.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "download_service.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

void DownloadService::AddDownloadTask(const DownloadTask& task,
                                      std::optional<DownloadItemCb> callback) {
  tasks.push_back(task);

  for (const auto& item : task.items) {
    StartDownloadItem(task.id, item, callback);
  }
}

void DownloadService::AddAsyncDownloadTask(
    const DownloadTask& task, std::optional<DownloadItemCb> callback) {
  tasks.push_back(task);

  for (const auto& item : task.items) {
    // TODO: maybe apply std::async is better?
    std::thread([this, task, &callback, item]() {
      this->StartDownloadItem(task.id, item, callback);
    }).detach();
  }
}

void DownloadService::StartDownloadItem(
    const std::string& downloadId, const DownloadItem& item,
    std::optional<DownloadItemCb> callback) {
  CTL_INF("Downloading item: " << downloadId);

  auto containerFolderPath{file_manager_utils::GetContainerFolderPath(
      file_manager_utils::downloadTypeToString(item.type))};
  CTL_INF("Container folder path: " << containerFolderPath.string() << "\n");

  auto itemFolderPath{containerFolderPath / std::filesystem::path(downloadId)};
  CTL_INF("itemFolderPath: " << itemFolderPath.string());
  if (!std::filesystem::exists(itemFolderPath)) {
    CTL_INF("Creating " << itemFolderPath.string());
    std::filesystem::create_directory(itemFolderPath);
  }

  auto outputFilePath{itemFolderPath / std::filesystem::path(item.fileName)};
  CTL_INF("Absolute file output: " << outputFilePath.string());

  uint64_t last = 0;
  uint64_t tot = 0;
  std::ofstream outputFile(outputFilePath, std::ios::binary);

  auto downloadUrl{item.host + "/" + item.path};
  CLI_LOG("Downloading url: " << downloadUrl);

  httplib::Client client(item.host);

  client.set_follow_location(true);
  client.Get(
      downloadUrl,
      [](const httplib::Response& res) {
        if (res.status != httplib::StatusCode::OK_200) {
          LOG_ERROR << "HTTP error: " << res.reason;
          return false;
        }
        return true;
      },
      [&](const char* data, size_t data_length) {
        tot += data_length;
        outputFile.write(data, data_length);
        return true;
      },
      [&item, &last, &outputFile, &callback, outputFilePath, this](
          uint64_t current, uint64_t total) {
        if (current - last > kUpdateProgressThreshold) {
          last = current;
          CLI_LOG("Downloading: " << current << " / " << total);
        }
        if (current == total) {
          outputFile.flush();
          outputFile.close();
          CLI_LOG("Done download: " << static_cast<double>(total) / 1024 / 1024
                                    << " MiB");
          if (callback.has_value()) {
            auto need_parse_gguf =
                item.path.find("cortexso") == std::string::npos;
            callback.value()(outputFilePath.string(), need_parse_gguf);
          }
          return false;
        }
        return true;
      });
}
