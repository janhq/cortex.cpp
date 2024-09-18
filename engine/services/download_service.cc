#include <curl/curl.h>
#include <httplib.h>
#include <stdio.h>
#include <trantor/utils/Logger.h>
#include <filesystem>
#include <thread>

#include "download_service.h"
#include "exceptions/failed_curl_exception.h"
#include "exceptions/failed_init_curl_exception.h"
#include "exceptions/failed_open_file_exception.h"
#include "utils/logging_utils.h"

namespace {
size_t WriteCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}
}  // namespace

void DownloadService::AddDownloadTask(
    const DownloadTask& task,
    std::optional<OnDownloadTaskSuccessfully> callback) {
  CLI_LOG("Validating download items, please wait..");
  // preprocess to check if all the item are valid
  auto total_download_size{0};
  for (const auto& item : task.items) {
    try {
      total_download_size += GetFileSize(item.downloadUrl);
    } catch (const FailedCurlException& e) {
      CTL_ERR("Found invalid download item: " << item.downloadUrl << " - "
                                              << e.what());
      throw;
    }
  }

  // all items are valid, start downloading
  for (const auto& item : task.items) {
    CLI_LOG("Start downloading: " + item.localPath.filename().string());
    Download(task.id, item);
  }

  if (callback.has_value()) {
    callback.value()(task);
  }
}

uint64_t DownloadService::GetFileSize(const std::string& url) const {
  CURL* curl;
  curl = curl_easy_init();

  if (!curl) {
    throw FailedInitCurlException();
  }

  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    // if we have a failed here. it meant the url is invalid
    throw FailedCurlException("CURL failed: " +
                              std::string(curl_easy_strerror(res)));
  }

  curl_off_t content_length = 0;
  res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                          &content_length);
  return content_length;
}

void DownloadService::AddAsyncDownloadTask(
    const DownloadTask& task,
    std::optional<OnDownloadTaskSuccessfully> callback) {

  for (const auto& item : task.items) {
    std::thread([this, task, &callback, item]() {
      this->Download(task.id, item);
    }).detach();
  }

  // TODO: how to call the callback when all the download has finished?
}

void DownloadService::Download(const std::string& download_id,
                               const DownloadItem& download_item) {
  CTL_INF("Absolute file output: " << download_item.localPath.string());

  CURL* curl;
  FILE* file;
  CURLcode res;

  curl = curl_easy_init();
  if (!curl) {
    throw FailedInitCurlException();
  }

  file = fopen(download_item.localPath.string().c_str(), "wb");
  if (!file) {
    auto err_msg{"Failed to open output file " +
                 download_item.localPath.string()};
    throw FailedOpenFileException(err_msg);
  }

  curl_easy_setopt(curl, CURLOPT_URL, download_item.downloadUrl.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }

  fclose(file);
  curl_easy_cleanup(curl);
}
