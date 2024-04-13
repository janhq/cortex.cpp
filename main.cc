#include "utils/nitro_utils.h"
#include <climits> // for PATH_MAX
#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>
#include <iostream>

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
  bool useHttps = false; // Default to HTTP
  std::string certPath;
  std::string keyPath;

  // Number of nitro threads
  if (argc > 1) {
    thread_num = std::atoi(argv[1]);
  }

  // Check for host argument
  if (argc > 2) {
    host = argv[2];
  }

  // Check for port argument
  if (argc > 3) {
    port = std::atoi(argv[3]); // Convert string argument to int
  }

  // Check for HTTPS argument
  if (argc > 5 && std::string(argv[4]) == "--https") {
    useHttps = true;
  }

  // Check for certificate and private key file paths
  if (argc > 6 && useHttps) {
    certPath = argv[6];
  }

  if (argc > 7 && useHttps) {
    keyPath = argv[7];
  }

  // Uploads folder path
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
  LOG_INFO << "Server started, listening at: " << host << ":" << port;
  LOG_INFO << "Please load your model";

  if (useHttps) {
    // Configure for HTTPS
    drogon::ssl::SSLServerContextConfig sslConfig;
    sslConfig.loadCertificate(certPath, keyPath);
    drogon::ssl::SSLServerContextPtr sslContext = std::make_shared<drogon::ssl::SSLServerContext>(sslConfig);
    drogon::app().addListener(host, port, sslContext);
    LOG_INFO << "Using HTTPS";
  } else {
    // Configure for HTTP
    drogon::app().addListener(host, port);
    LOG_INFO << "Using HTTP";
  }

  drogon::app().setThreadNum(drogon_thread_num);

  if (!uploads_folder_path.empty()) {
    LOG_INFO << "Drogon uploads folder is at: " << uploads_folder_path;
    drogon::app().setUploadPath(uploads_folder_path);
  }

  LOG_INFO << "Number of threads: " << drogon::app().getThreadNum();
  drogon::app().run();

  return 0;
}
