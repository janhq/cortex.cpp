#include "utils/nitro_utils.h"
#include <climits> // for PATH_MAX
#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <iostream>
#include <stdexcept> // for std::runtime_error

#if defined(__APPLE__) && defined(__MACH__)
#include <libgen.h> // for dirname()
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <libgen.h> // for dirname()
#include <unistd.h> // for readlink()
#elif defined(_WIN32)
#include <windows.h>
#undef max
#else
#error "Unsupported platform!"
#endif

int main(int argc, char *argv[]) {
  int thread_num = 1;
  std::string host = "127.0.0.1";
  int port = 3928;
  std::string uploads_folder_path;

  // Parse command-line arguments
  if (argc > 1) {
    thread_num = std::atoi(argv[1]);
  }
  if (argc > 2) {
    host = argv[2];
  }
  if (argc > 3) {
    port = std::atoi(argv[3]);
  }
  if (argc > 4) {
    uploads_folder_path = argv[4];
  }

  int logical_cores = std::thread::hardware_concurrency();
  int drogon_thread_num = std::max(thread_num, logical_cores);
  nitro_utils::nitro_logo();

#ifdef NITRO_VERSION
  LOG_INFO << "Nitro version: " << NITRO_VERSION;
#else
  LOG_INFO << "Nitro version: undefined";
#endif

  // Attempt to add listener and catch exception if port is already in use
  try {
    drogon::app().addListener(host, port);
  } catch (const std::exception &e) {
    std::cerr << "Failed to bind to port " << port << ": " << e.what()
              << std::endl;
    return 1; // Exit with error code
  }

  drogon::app().setThreadNum(drogon_thread_num);
  if (!uploads_folder_path.empty()) {
    LOG_INFO << "Drogon uploads folder is at: " << uploads_folder_path;
    drogon::app().setUploadPath(uploads_folder_path);
  }
  LOG_INFO << "Number of threads is: " << drogon::app().getThreadNum();
  LOG_INFO << "Server started, listening at: " << host << ":" << port;
  LOG_INFO << "Please load your model";

  drogon::app().run();

  return 0;
}
