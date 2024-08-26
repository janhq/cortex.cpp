#pragma once

#include <optional>
#include <vector>
#include "httplib.h"

enum class DownloadType { Model, Engine, Miscellaneous };

enum class DownloadStatus {
  Pending,
  Downloading,
  Error,
  Downloaded,
};

struct DownloadItem {
  std::string id;

  std::string host;

  std::string fileName;

  DownloadType type;

  std::string path;

  uint64_t totalSize;

  uint64_t transferredSize;

  DownloadStatus status;

  std::optional<std::string> checksum;
};

struct DownloadTask {
  std::string id;
  DownloadType type;
  float percentage;
  DownloadStatus status;
  std::optional<std::string> error;
  std::vector<DownloadItem> items;
};

class DownloadService {
 public:
  /**
  * @brief Synchronously download.
  * 
  * @param task 
  */
  using DownloadItemCb = std::function<void(const std::string&)>;
  void AddDownloadTask(const DownloadTask& task);

  void AddAsyncDownloadTask(const DownloadTask& task);

  // TODO: [NamH] implement the following methods
  //  void removeTask(const std::string &id);
  //  void registerCallback
  //  setup folder path at runtime
  //  register action after downloaded

 private:
  void StartDownloadItem(const std::string& downloadId,
                         const DownloadItem& item,
                         const DownloadItemCb& callback = nullptr);

  const std::string GetContainerFolderPath(DownloadType type);

  // store tasks so we can abort it later
  std::vector<DownloadTask> tasks;
  const int kUpdateProgressThreshold = 100000000;
};