#pragma once
#include <drogon/HttpClient.h>
#include <drogon/HttpResponse.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <ostream>
#include <random>
#include <regex>
#include <string>
#include <vector>
#if defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

#if __APPLE__
#include <mach-o/dyld.h>
#endif

namespace cortex_utils {
inline std::string logs_folder = "./logs";
inline std::string logs_base_name = "./logs/cortex.log";
inline std::string logs_cli_base_name = "./logs/cortex-cli.log";

inline std::string rtrim(const std::string& str) {
  size_t end = str.find_last_not_of("\n\t ");
  return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

inline drogon::HttpResponsePtr CreateCortexHttpResponse() {
  return drogon::HttpResponse::newHttpResponse();
}

inline drogon::HttpResponsePtr CreateCortexHttpJsonResponse(
    const Json::Value& data) {
  return drogon::HttpResponse::newHttpJsonResponse(data);
};

inline drogon::HttpResponsePtr CreateCortexStreamResponse(
    const std::function<std::size_t(char*, std::size_t)>& callback,
    const std::string& attachmentFileName = "") {
  return drogon::HttpResponse::newStreamResponse(
      callback, attachmentFileName, drogon::CT_NONE, "text/event-stream");
}

#if defined(_WIN32)
inline std::string GetCurrentPath() {
  wchar_t path[MAX_PATH];
  DWORD result = GetModuleFileNameW(NULL, path, MAX_PATH);
  if (result == 0) {
    std::wcerr << L"Error getting module file name." << std::endl;
    return "";
  }
  std::wstring::size_type pos = std::wstring(path).find_last_of(L"\\/");
  auto ws = std::wstring(path).substr(0, pos);
  std::string res;
  std::transform(ws.begin(), ws.end(), std::back_inserter(res),
                 [](wchar_t c) { return (char)c; });
  return res;
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
  if (len == -1 || len == buf.size()) {
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
