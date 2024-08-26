#include <trantor/utils/Logger.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "download_service.h"

void DownloadService::AddDownloadTask(const DownloadTask& task) {
  tasks.push_back(task);

  for (const auto& item : task.items) {
    StartDownloadItem(task.id, item);
  }
}

void DownloadService::AddAsyncDownloadTask(const DownloadTask& task) {
  tasks.push_back(task);
  for (const auto& item : task.items) {
    // TODO: maybe apply std::async is better?
    std::thread([this, task, item]() {
      this->StartDownloadItem(task.id, item);
    }).detach();
  }
}

const std::string DownloadService::GetContainerFolderPath(DownloadType type) {
  std::filesystem::path container_folder_path;

  switch (type) {
    case DownloadType::Model: {
      container_folder_path = std::filesystem::current_path() / "models";
      break;
    }
    case DownloadType::Engine: {
      container_folder_path = std::filesystem::current_path() / "engines";
      break;
    }
    default: {
      container_folder_path = std::filesystem::current_path() / "misc";
      break;
    }
  }

  if (!std::filesystem::exists(container_folder_path)) {
    LOG_INFO << "Creating folder: " << container_folder_path.string() << "\n";
    std::filesystem::create_directory(container_folder_path);
  }

  return container_folder_path.string();
}

void DownloadService::StartDownloadItem(const std::string& downloadId,
                                        const DownloadItem& item,
                                        const DownloadItemCb& callback) {
  LOG_INFO << "Downloading item: " << downloadId;
  const std::string containerFolderPath = GetContainerFolderPath(item.type);
  LOG_INFO << "Container folder path: " << containerFolderPath << "\n";
  const std::filesystem::path itemFolderPath =
      std::filesystem::path(containerFolderPath) /
      std::filesystem::path(downloadId);
  if (!std::filesystem::exists(itemFolderPath)) {
    LOG_INFO << "Creating " << itemFolderPath.string();
    std::filesystem::create_directory(itemFolderPath);
  }

  LOG_INFO << "itemFolderPath: " << itemFolderPath.string();
  auto outputFilePath = itemFolderPath / std::filesystem::path(item.fileName);
  LOG_INFO << "Absolute file output: " << outputFilePath.string();

  uint64_t last = 0;
  uint64_t tot = 0;
  std::ofstream outputFile(outputFilePath, std::ios::binary);

  std::ostringstream downloadUrl;
  downloadUrl << item.host << "/" << item.path;
  LOG_INFO << "Downloading url: " << downloadUrl.str();

  httplib::Client client(item.host);

  client.set_follow_location(true);
  client.Get(
      downloadUrl.str(),
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
      [&last, this](uint64_t current, uint64_t total) {
        if (current - last > kUpdateProgressThreshold) {
          last = current;
          LOG_INFO << "Downloading: " << current << " / " << total;
        }
        if (current == total) {
          LOG_INFO << "Done download: "
                   << static_cast<double>(total) / 1024 / 1024 << " MiB";
          return false;
        }
        return true;
      });
  if(callback){
    callback(outputFilePath.string());
  }
}