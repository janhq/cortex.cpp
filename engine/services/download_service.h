#pragma once

#include <curl/curl.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <sstream>
#include <vector>
#include "utils/result.hpp"

enum class DownloadType { Model, Engine, Miscellaneous, CudaToolkit, Cortex };

struct DownloadItem {
  std::string id;

  std::string downloadUrl;

  /**
   * An absolute path to where the file is located (locally).
   */
  std::filesystem::path localPath;

  std::optional<std::string> checksum;

  std::optional<uint64_t> bytes;

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
      DownloadTask& task,
      std::optional<OnDownloadTaskSuccessfully> callback = std::nullopt);

  void AddAsyncDownloadTask(
      const DownloadTask& task,
      std::optional<OnDownloadTaskSuccessfully> callback = std::nullopt);

  /**
   * Getting file size for a provided url. Can be used to validating the download url.
   *
   * @param url - url to get file size
   */
  cpp::result<uint64_t, std::string> GetFileSize(
      const std::string& url) const noexcept;

 private:
  cpp::result<void, std::string> Download(const std::string& download_id,
                                          const DownloadItem& download_item,
                                          bool allow_resume) noexcept;

  curl_off_t GetLocalFileSize(const std::filesystem::path& path) const;
};
