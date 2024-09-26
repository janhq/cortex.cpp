#include "download_service.h"
#include <curl/curl.h>
#include <httplib.h>
#include <stdio.h>
#include <trantor/utils/Logger.h>
#include <filesystem>
#include <optional>
#include <ostream>
#include <thread>
#include "download_service.h"
#include "utils/format_utils.h"
#include "utils/logging_utils.h"
#include "utils/result.hpp"

#ifdef _WIN32
#define ftell64(f) _ftelli64(f)
#define fseek64(f, o, w) _fseeki64(f, o, w)
#else
#define ftell64(f) ftello(f)
#define fseek64(f, o, w) fseeko(f, o, w)
#endif

namespace {
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
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

cpp::result<void, std::string> DownloadService::AddDownloadTask(
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
  for (const auto& item : task.items) {
    CLI_LOG("Start downloading: " + item.localPath.filename().string());
    auto result = Download(task.id, item, true);
    if (result.has_error()) {
      dl_err_msg = result.error();
      break;
    }
  }
  if (dl_err_msg.has_value()) {
    CTL_ERR(dl_err_msg.value());
    return cpp::fail(dl_err_msg.value());
  }

  if (callback.has_value()) {
    callback.value()(task);
  }
  return {};
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

cpp::result<void, std::string> DownloadService::AddAsyncDownloadTask(
    DownloadTask& task,
    std::optional<OnDownloadTaskSuccessfully> callback) noexcept {
  auto verifying_result = VerifyDownloadTask(task);
  if (verifying_result.has_error()) {
    return cpp::fail(verifying_result.error());
  }

  auto execute_download_async = [&, task, callback]() {
    std::optional<std::string> dl_err_msg = std::nullopt;
    for (const auto& item : task.items) {
      CTL_INF("Start downloading: " + item.localPath.filename().string());
      auto result = Download(task.id, item, false);
      if (result.has_error()) {
        dl_err_msg = result.error();
        break;
      }
    }

    if (dl_err_msg.has_value()) {
      CTL_ERR(dl_err_msg.value());
      return;
    }

    if (callback.has_value()) {
      CTL_INF("Download success, executing post download lambda!");
      callback.value()(task);
    }
  };

  std::thread t(execute_download_async);
  t.detach();

  return {};
}

cpp::result<void, std::string> DownloadService::Download(
    const std::string& download_id, const DownloadItem& download_item,
    bool allow_resume) noexcept {
  CTL_INF("Absolute file output: " << download_item.localPath.string());

  CURL* curl;
  FILE* file;
  CURLcode res;

  curl = curl_easy_init();
  if (!curl) {
    return cpp::fail(static_cast<std::string>("Failed to init CURL"));
  }

  std::string mode = "wb";
  if (allow_resume && std::filesystem::exists(download_item.localPath) &&
      download_item.bytes.has_value()) {
    curl_off_t existing_file_size = GetLocalFileSize(download_item.localPath);
    if (existing_file_size == -1) {
      CLI_LOG("Cannot get file size: " << download_item.localPath.string()
                                       << " . Start download over!");
    } else {
      CTL_INF("Existing file size: " << download_item.downloadUrl << " - "
                                     << download_item.localPath.string()
                                     << " - " << existing_file_size);
      auto missing_bytes = download_item.bytes.value() - existing_file_size;
      if (missing_bytes > 0) {
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
          return {};
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
  return {};
}

curl_off_t DownloadService::GetLocalFileSize(
    const std::filesystem::path& path) const {
  FILE* file = fopen(path.string().c_str(), "r");
  if (!file) {
    return -1;
  }

  if (fseek64(file, 0, SEEK_END) != 0) {
    return -1;
  }

  curl_off_t file_size = ftell64(file);
  fclose(file);
  return file_size;
}
