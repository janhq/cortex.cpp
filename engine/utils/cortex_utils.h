#pragma once

#include <drogon/HttpClient.h>
#include <drogon/HttpResponse.h>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>
#include <string>
#include <utility>
#if defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <codecvt>
#include <locale>
#endif

namespace cortex_utils {
inline std::string logs_folder = "./logs";
inline std::string logs_base_name = "./logs/cortex.log";
inline std::string logs_cli_base_name = "./logs/cortex-cli.log";

// example: Mon, 25 Nov 2024 09:57:03 GMT
inline std::string GetDateRFC1123() {
  std::time_t now = std::time(nullptr);
#ifdef _MSC_VER
  std::tm gmt_time = {};
  gmtime_s(&gmt_time, &now);
  std::ostringstream oss;
  oss << std::put_time(&gmt_time, "%a, %d %b %Y %H:%M:%S GMT");
#else
  std::tm* gmt_time_ptr = std::gmtime(&now);
  std::ostringstream oss;
  oss << std::put_time(gmt_time_ptr, "%a, %d %b %Y %H:%M:%S GMT");
#endif
  return oss.str();
}

inline drogon::HttpResponsePtr CreateCortexHttpResponse() {
  auto res = drogon::HttpResponse::newHttpResponse();
#if defined(_WIN32)
  res->addHeader("date", GetDateRFC1123());
#endif
  return res;
}

inline drogon::HttpResponsePtr CreateCortexHttpTextAsJsonResponse(
    const std::string& data) {
  auto res = drogon::HttpResponse::newHttpResponse();
  res->setBody(data);
  res->setContentTypeCode(drogon::CT_APPLICATION_JSON);
#if defined(_WIN32)
  res->addHeader("date", GetDateRFC1123());
#endif
  return res;
};

inline drogon::HttpResponsePtr CreateCortexHttpJsonResponse(
    const Json::Value& data) {
  auto res = drogon::HttpResponse::newHttpJsonResponse(data);
#if defined(_WIN32)
  res->addHeader("date", GetDateRFC1123());
#endif
  return res;
};

inline drogon::HttpResponsePtr CreateCortexContentResponse(
    std::pair<std::unique_ptr<char[]>, size_t> content) {
  auto [buffer, size] = std::move(content);
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(std::string(buffer.get(), size));
  resp->setContentTypeCode(drogon::CT_APPLICATION_OCTET_STREAM);

#if defined(_WIN32)
  resp->addHeader("date", GetDateRFC1123());
#endif
  return resp;
}

inline drogon::HttpResponsePtr CreateTextPlainResponse(
    const std::string& text) {
  auto resp = drogon::HttpResponse::newHttpResponse();
  resp->setBody(text);
  resp->setContentTypeCode(drogon::CT_TEXT_PLAIN);
#if defined(_WIN32)
  resp->addHeader("date", GetDateRFC1123());
#endif
  return resp;
}

inline drogon::HttpResponsePtr CreateCortexStreamResponse(
    const std::function<std::size_t(char*, std::size_t)>& callback,
    const std::string& attachmentFileName = "") {
  auto res = drogon::HttpResponse::newStreamResponse(
      callback, attachmentFileName, drogon::CT_NONE, "text/event-stream");
#if defined(_WIN32)
  res->addHeader("date", GetDateRFC1123());
#endif
  return res;
}

#if defined(_WIN32)
inline std::string GetCurrentPath() {
  char path[MAX_PATH];
  DWORD result = GetModuleFileNameA(NULL, path, MAX_PATH);
  if (result == 0) {
    std::cerr << "Error getting module file name." << std::endl;
    return "";
  }

  std::string::size_type pos = std::string(path).find_last_of("\\/");
  return std::string(path).substr(0, pos);
}
#else
inline std::string GetCurrentPath() {
#ifdef __APPLE__
  char buf[PATH_MAX];
  uint32_t bufsize = PATH_MAX;

  if (_NSGetExecutablePath(buf, &bufsize) == 0) {
    auto s = std::string(buf);
    auto const pos = s.find_last_of('/');
    return s.substr(0, pos);
  }
  return "";
#else
  std::vector<char> buf(PATH_MAX);
  ssize_t len = readlink("/proc/self/exe", &buf[0], buf.size());
  if (len == -1 || len == (ssize_t) buf.size()) {
    std::cerr << "Error reading symlink /proc/self/exe." << std::endl;
    return "";
  }
  auto s = std::string(&buf[0], len);
  auto const pos = s.find_last_of('/');
  return s.substr(0, pos);
#endif
}
#endif
}  // namespace cortex_utils
