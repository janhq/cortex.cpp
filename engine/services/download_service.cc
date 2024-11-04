#include <curl/curl.h>
#include <httplib.h>
#include <stdio.h>
#include <filesystem>
#include <mutex>
#include <optional>
#include <ostream>
#include <utility>
#include "download_service.h"
#include "utils/format_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"
#include "utils/url_parser.h"

#ifdef _WIN32
#define ftell64(f) _ftelli64(f)
#define fseek64(f, o, w) _fseeki64(f, o, w)
#else
#define ftell64(f) ftello(f)
#define fseek64(f, o, w) fseeko(f, o, w)
#endif

namespace {
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  size_t written = fwrite(ptr, size, nmemb, (FILE*)userdata);
  return written;
}

inline curl_slist* CreateHeaders(const std::string& url) {
  try {
    auto url_obj = url_parser::FromUrlString(url);
    if (url_obj.host == huggingface_utils::kHuggingfaceHost) {
      return huggingface_utils::CreateCurlHfHeaders();
    } else {
      return nullptr;
    }
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return nullptr;
  }
}
}  // namespace

cpp::result<void, std::string> DownloadService::VerifyDownloadTask(
    DownloadTask& task) const noexcept {
  CLI_LOG("Validating download items, please wait..");

  auto total_download_size{0};
  std::optional<std::string> err_msg = std::nullopt;

  for (auto& item : task.items) {
    auto file_size = GetFileSize(item.downloadUrl);
    if (file_size.has_error()) {
      err_msg = file_size.error();
      break;
    }

    item.bytes = file_size.value();
    total_download_size += file_size.value();
  }

  if (err_msg.has_value()) {
    CTL_ERR(err_msg.value());
    return cpp::fail(err_msg.value());
  }

  return {};
}

cpp::result<bool, std::string> DownloadService::AddDownloadTask(
    DownloadTask& task,
    std::optional<OnDownloadTaskSuccessfully> callback) noexcept {
  auto validating_result = VerifyDownloadTask(task);
  if (validating_result.has_error()) {
    return cpp::fail(validating_result.error());
  }

  // all items are valid, start downloading
  // if any item from the task failed to download, the whole task will be
  // considered failed
  std::optional<std::string> dl_err_msg = std::nullopt;
  bool has_task_done = false;
  for (const auto& item : task.items) {
    CLI_LOG("Start downloading: " + item.localPath.filename().string());
    auto result = Download(task.id, item);
    if (result.has_error()) {
      dl_err_msg = result.error();
      break;
    } else if (result) {
      has_task_done |= result.value();
    }
  }
  if (dl_err_msg.has_value()) {
    // CTL_ERR(dl_err_msg.value());
    return cpp::fail(dl_err_msg.value());
  }

  if (callback.has_value()) {
    callback.value()(task);
  }
  return has_task_done;
}

cpp::result<uint64_t, std::string> DownloadService::GetFileSize(
    const std::string& url) const noexcept {
  CURL* curl;
  curl = curl_easy_init();

  if (!curl) {
    return cpp::fail(static_cast<std::string>("Failed to init CURL"));
  }

  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  if (auto headers = CreateHeaders(url); headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    return cpp::fail(static_cast<std::string>(
        "CURL failed: " + std::string(curl_easy_strerror(res))));
  }

  curl_off_t content_length = 0;
  res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                          &content_length);
  return content_length;
}

cpp::result<bool, std::string> DownloadService::Download(
    const std::string& download_id,
    const DownloadItem& download_item) noexcept {
  CTL_INF("Absolute file output: " << download_item.localPath.string());

  CURL* curl;
  FILE* file;
  CURLcode res;

  curl = curl_easy_init();
  if (!curl) {
    return cpp::fail(static_cast<std::string>("Failed to init CURL"));
  }

  std::string mode = "wb";
  if (std::filesystem::exists(download_item.localPath) &&
      download_item.bytes.has_value()) {
    curl_off_t existing_file_size = GetLocalFileSize(download_item.localPath);
    if (existing_file_size == -1) {
      CLI_LOG("Cannot get file size: " << download_item.localPath.string()
                                       << " . Start download over!");
    } else {
      CTL_INF("Existing file size: " << download_item.downloadUrl << " - "
                                     << download_item.localPath.string()
                                     << " - " << existing_file_size);
      CTL_INF("Download item size: " << download_item.bytes.value());
      auto missing_bytes = download_item.bytes.value() - existing_file_size;
      if (missing_bytes > 0 &&
          download_item.localPath.extension().string() != ".yaml" &&
          download_item.localPath.extension().string() != ".yml") {
        CLI_LOG("Found unfinished download! Additional "
                << format_utils::BytesToHumanReadable(missing_bytes)
                << " need to be downloaded.");
        std::cout << "Continue download [Y/n]: " << std::flush;
        std::string answer{""};
        std::getline(std::cin, answer);
        if (answer == "Y" || answer == "y" || answer.empty()) {
          mode = "ab";
          CLI_LOG("Resuming download..");
        } else {
          CLI_LOG("Start over..");
        }
      } else {
        CLI_LOG(download_item.localPath.filename().string()
                << " is already downloaded!");
        std::cout << "Re-download? [Y/n]: " << std::flush;

        std::string answer = "";
        std::getline(std::cin, answer);
        if (answer == "Y" || answer == "y" || answer.empty()) {
          CLI_LOG("Re-downloading..");
        } else {
          return false;
        }
      }
    }
  }

  file = fopen(download_item.localPath.string().c_str(), mode.c_str());
  if (!file) {
    return cpp::fail("Failed to open output file " +
                     download_item.localPath.string());
  }

  curl_easy_setopt(curl, CURLOPT_URL, download_item.downloadUrl.c_str());
  if (auto headers = CreateHeaders(download_item.downloadUrl); headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  if (mode == "ab") {
    auto local_file_size = GetLocalFileSize(download_item.localPath);
    if (local_file_size != -1) {
      curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE,
                       GetLocalFileSize(download_item.localPath));
    } else {
      CTL_ERR("Cannot get file size: " << download_item.localPath.string());
    }
  }

  res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    return cpp::fail("Download failed! Error: " +
                     static_cast<std::string>(curl_easy_strerror(res)));
  }

  fclose(file);
  curl_easy_cleanup(curl);
  return true;
}

curl_off_t DownloadService::GetLocalFileSize(
    const std::filesystem::path& path) const {
  auto file = fopen(path.string().c_str(), "r");
  if (!file) {
    return -1;
  }

  if (fseek64(file, 0, SEEK_END) != 0) {
    return -1;
  }

  auto file_size = ftell64(file);
  fclose(file);
  return file_size;
}

void DownloadService::WorkerThread() {
  while (!stop_flag_) {
    DownloadTask task;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      queue_condition_.wait(
          lock, [this] { return !task_queue_.empty() || stop_flag_; });
      if (stop_flag_) {
        CTL_INF("Stopping download service..");
        break;
      }
      task = std::move(task_queue_.front());
      task_queue_.pop();
    }
    ProcessTask(task);
  }
}

void DownloadService::ProcessTask(DownloadTask& task) {
  CTL_INF("Processing task: " + task.id);
  std::vector<std::pair<CURL*, FILE*>> task_handles;

  active_task_ = std::make_shared<DownloadTask>(task);

  for (auto& item : task.items) {
    CURL* handle = curl_easy_init();
    if (handle == nullptr) {
      // skip the task
      CTL_ERR("Failed to init curl!");
      return;
    }

    FILE* file;
    file = fopen(item.localPath.string().c_str(), "wb");
    if (!file) {
      CTL_ERR("Failed to open output file " + item.localPath.string());
      return;
    }

    auto dl_data_ptr = std::make_shared<DownloadingData>(DownloadingData{
        .item_id = item.id,
        .download_service = this,
    });
    downloading_data_map_.insert(std::make_pair(item.id, dl_data_ptr));

    if (auto headers = CreateHeaders(item.downloadUrl); headers) {
      curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
    }
    curl_easy_setopt(handle, CURLOPT_URL, item.downloadUrl.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(handle, CURLOPT_XFERINFODATA, dl_data_ptr.get());

    curl_multi_add_handle(multi_handle_, handle);
    task_handles.push_back(std::make_pair(handle, file));
    CTL_INF("Adding item to multi curl: " + item.ToString());
  }

  event_queue_->enqueue(
      EventType::DownloadEvent,
      DownloadEvent{.type_ = DownloadEventType::DownloadStarted,
                    .download_task_ = task});

  int still_running = 0;
  bool is_terminated = false;
  do {
    curl_multi_perform(multi_handle_, &still_running);
    curl_multi_wait(multi_handle_, NULL, 0, MAX_WAIT_MSECS, NULL);

    if (IsTaskTerminated(task.id) || stop_flag_) {
      CTL_INF("IsTaskTerminated " + std::to_string(IsTaskTerminated(task.id)));
      CTL_INF("stop_flag_ " + std::to_string(stop_flag_));

      is_terminated = true;
      break;
    }
  } while (still_running);

  if (stop_flag_) {
    CTL_INF("Download service is stopping..");

    // try to close file
    for (auto pair : task_handles) {
      fclose(pair.second);
    }

    active_task_.reset();
    downloading_data_map_.clear();
    return;
  }

  ProcessCompletedTransfers();
  for (auto pair : task_handles) {
    curl_multi_remove_handle(multi_handle_, pair.first);
    curl_easy_cleanup(pair.first);
    fclose(pair.second);
  }
  downloading_data_map_.clear();
  active_task_.reset();

  RemoveTaskFromStopList(task.id);

  // if terminate by API calling and not from process stopping, we emit
  // DownloadStopped event
  if (is_terminated) {
    event_queue_->enqueue(
        EventType::DownloadEvent,
        DownloadEvent{.type_ = DownloadEventType::DownloadStopped,
                      .download_task_ = task});
  } else {
    CTL_INF("Executing callback..");
    ExecuteCallback(task);

    event_queue_->enqueue(
        EventType::DownloadEvent,
        DownloadEvent{.type_ = DownloadEventType::DownloadSuccess,
                      .download_task_ = task});
  }
}

cpp::result<std::string, std::string> DownloadService::StopTask(
    const std::string& task_id) {
  std::lock_guard<std::mutex> lock(stop_mutex_);

  tasks_to_stop_.insert(task_id);
  CTL_INF("Added task to stop list: " << task_id);
  return task_id;
}

void DownloadService::ProcessCompletedTransfers() {
  CURLMsg* msg;
  int remaining_msg_count;

  while ((msg = curl_multi_info_read(multi_handle_, &remaining_msg_count))) {
    if (msg->msg == CURLMSG_DONE) {
      auto handle = msg->easy_handle;

      auto return_code = msg->data.result;
      char* url;
      curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &url);
      if (return_code != CURLE_OK) {
        CTL_ERR("Download failed for: " << url << " - "
                                        << curl_easy_strerror(return_code));
        continue;
      }

      auto http_status_code = 0;
      curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &http_status_code);
      if (http_status_code == 200) {
        CTL_INF("Download completed successfully for: " << url);
      } else {
        CTL_ERR("Download failed for: " << url << " - HTTP status code: "
                                        << http_status_code);
      }
    }
  }
}

cpp::result<DownloadTask, std::string> DownloadService::AddTask(
    DownloadTask& task, std::function<void(const DownloadTask&)> callback) {
  auto validate_result = VerifyDownloadTask(task);
  if (validate_result.has_error()) {
    return cpp::fail(validate_result.error());
  }

  {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    callbacks_[task.id] = std::move(callback);
  }

  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    task_queue_.push(task);
    CTL_INF("Task added to queue: " << task.id);
  }

  queue_condition_.notify_one();
  return task;
}

bool DownloadService::IsTaskTerminated(const std::string& task_id) {
  // can use shared mutex lock here?
  std::lock_guard<std::mutex> lock(stop_mutex_);
  return tasks_to_stop_.find(task_id) != tasks_to_stop_.end();
}

void DownloadService::RemoveTaskFromStopList(const std::string& task_id) {
  std::lock_guard<std::mutex> lock(stop_mutex_);
  tasks_to_stop_.erase(task_id);
}

void DownloadService::ExecuteCallback(const DownloadTask& task) {
  std::lock_guard<std::mutex> lock(callbacks_mutex_);
  auto it = callbacks_.find(task.id);
  if (it != callbacks_.end()) {
    it->second(task);
    callbacks_.erase(it);
  }
}
