#pragma once

#include <curl/curl.h>
#include <eventpp/eventqueue.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <vector>
#include "common/download_event.h"
#include "utils/result.hpp"

class DownloadService {
 public:
  using DownloadEvent = cortex::event::DownloadEvent;
  using EventQueue = eventpp::EventQueue<std::string, void(DownloadEvent)>;

  explicit DownloadService() = default;

  explicit DownloadService(std::shared_ptr<EventQueue> event_queue)
      : event_queue_{event_queue} {};

  using OnDownloadTaskSuccessfully =
      std::function<void(const DownloadTask& task)>;

  cpp::result<bool, std::string> AddDownloadTask(
      DownloadTask& task, std::optional<OnDownloadTaskSuccessfully> callback =
                              std::nullopt) noexcept;

  cpp::result<bool, std::string> AddAsyncDownloadTask(
      DownloadTask& task, std::optional<OnDownloadTaskSuccessfully> callback =
                              std::nullopt) noexcept;

  /**
   * Getting file size for a provided url. Can be used to validating the download url.
   *
   * @param url - url to get file size
   */
  cpp::result<uint64_t, std::string> GetFileSize(
      const std::string& url) const noexcept;

 private:
  cpp::result<void, std::string> VerifyDownloadTask(
      DownloadTask& task) const noexcept;

  cpp::result<bool, std::string> Download(const std::string& download_id,
                                          const DownloadItem& download_item,
                                          bool allow_resume) noexcept;

  curl_off_t GetLocalFileSize(const std::filesystem::path& path) const;

  std::shared_ptr<EventQueue> event_queue_;

  std::vector<std::string> download_task_list_;
  std::unordered_map<std::string, DownloadTask> download_task_map_;

  std::optional<std::string> active_download_task_id_;
  std::optional<std::string> active_download_item_id_;

  /**
   * Invoked when download is completed (both failed or success)
   */
  void CleanUp(const std::string& task_id);

  static int ProgressCallback(void* ptr, double dltotal, double dlnow,
                              double ultotal, double ulnow) {
    auto service = static_cast<DownloadService*>(ptr);
    if (service->event_queue_ == nullptr) {
      return 0;
    }

    auto active_task_id = service->active_download_task_id_;
    auto active_item_id = service->active_download_item_id_;
    if (!active_task_id.has_value() || !active_item_id.has_value()) {
      return 0;
    }

    auto task = service->download_task_map_[active_task_id.value()];

    // loop through download items, find the active one and update it
    for (auto& item : task.items) {
      if (item.id == active_item_id.value()) {
        item.downloadedBytes = dlnow;
        item.bytes = dltotal;
        break;
      }
    }

    service->event_queue_->enqueue(
        "download-update",
        DownloadEvent{
            .type_ = cortex::event::DownloadEventType::DownloadUpdated,
            .download_task_ = task});
    return 0;
  }
};
