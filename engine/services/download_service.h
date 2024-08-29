#pragma once

#include <functional>
#include <optional>
#include <vector>

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

  std::optional<std::string> checksum;
};

struct DownloadTask {
  std::string id;
  DownloadType type;
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
  using DownloadItemCb = std::function<void(const std::string&, bool)>;
  void AddDownloadTask(const DownloadTask& task,
                       std::optional<DownloadItemCb> callback = std::nullopt);

  void AddAsyncDownloadTask(
      const DownloadTask& task,
      std::optional<DownloadItemCb> callback = std::nullopt);

  // TODO: [NamH] implement the following methods
  //  void removeTask(const std::string &id);
  //  void registerCallback
  //  setup folder path at runtime
  //  register action after downloaded

 private:
  void StartDownloadItem(const std::string& downloadId,
                         const DownloadItem& item,
                         std::optional<DownloadItemCb> callback = std::nullopt);

  // store tasks so we can abort it later
  std::vector<DownloadTask> tasks;
  const int kUpdateProgressThreshold = 100000000;
};