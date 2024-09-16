#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <sstream>
#include <vector>

enum class DownloadType { Model, Engine, Miscellaneous, CudaToolkit, Cortex };

struct DownloadItem {
  std::string id;

  std::string downloadUrl;

  /**
   * An absolute path to where the file is located (locally).
   */
  std::filesystem::path localPath;

  std::optional<std::string> checksum;

  std::string ToString() const {
    std::ostringstream output;
    output << "DownloadItem{id: " << id << ", downloadUrl: " << downloadUrl
           << ", localContainerPath: " << localPath
           << ", checksum: " << checksum.value_or("N/A") << "}";
    return output.str();
  }
};

struct DownloadTask {
  std::string id;

  DownloadType type;

  std::vector<DownloadItem> items;

  std::string ToString() const {
    std::ostringstream output;
    output << "DownloadTask{id: " << id << ", type: " << static_cast<int>(type)
           << ", items: [";
    for (const auto& item : items) {
      output << item.ToString() << ", ";
    }
    output << "]}";
    return output.str();
  }
};

class DownloadService {
 public:
  using OnDownloadTaskSuccessfully =
      std::function<void(const DownloadTask& task)>;

  void AddDownloadTask(
      const DownloadTask& task,
      std::optional<OnDownloadTaskSuccessfully> callback = std::nullopt);

  void AddAsyncDownloadTask(
      const DownloadTask& task,
      std::optional<OnDownloadTaskSuccessfully> callback = std::nullopt);

  /**
   * Getting file size for a provided url.
   *
   * @param url - url to get file size
   */
  uint64_t GetFileSize(const std::string& url) const;

 private:
  void Download(const std::string& download_id,
                const DownloadItem& download_item);
};
