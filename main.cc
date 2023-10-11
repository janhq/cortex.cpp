#include "controllers/nitro_utils.h"
#include <climits> // for PATH_MAX
#include <drogon/drogon.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <libgen.h> // for dirname()
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <libgen.h> // for dirname()
#include <unistd.h> // for readlink()
#elif defined(_WIN32)
#include <windows.h>
#else
#error "Unsupported platform!"
#endif

int main(int argc, char *argv[]) {

  std::string host = "127.0.0.1";
  int port = 3928;

  // Check for host argument
  if (argc > 1) {
    host = argv[1];
  }

  // Check for port argument
  if (argc > 2) {
    port = std::atoi(argv[2]); // Convert string argument to int
  }

  nitro_utils::nitro_logo();
  LOG_INFO << "Server started, listening at: " << host << ":" << port
           << "please load your model";
  drogon::app().addListener(host, port);
  drogon::app().run();

  return 0;
}
