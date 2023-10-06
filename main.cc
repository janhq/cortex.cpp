
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

int main() {
  std::string configPath;

#if defined(__APPLE__) && defined(__MACH__)
  char path[PATH_MAX];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0) {
    path[size] = '\0'; // Null-terminate the string
    char *dir = dirname(path);
    configPath = std::string(dir) + "/config/config.json";
  } else {
    LOG_ERROR << "Failed to get binary location!";
    return 1;
  }
#elif defined(__linux__)
  char path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len != -1) {
    path[len] = '\0';
    char *dir = dirname(path);
    configPath = std::string(dir) + "/config/config.json";
  } else {
    LOG_ERROR << "Failed to get binary location!";
    return 1;
  }
#elif defined(_WIN32)
  char path[MAX_PATH];
  char dir[MAX_PATH];
  // char dir[MAX_PATH];
  if(GetModuleFileNameA(NULL, path, sizeof(path))) {
      char* lastBackslash = strrchr(path, '\\');
      if (lastBackslash == nullptr) {
        return 1;
      }
      lastBackslash[0] = '\0';
      strcpy(dir, path);
      configPath = std::string(dir) + "/config/config.json";
  }
  else {
    LOG_ERROR << "Failed to get binary location!";
    return 1;
  }
#else
  LOG_ERROR << "Unsupported platform!";
  return 1;
#endif

  // Set HTTP listener address and port
  drogon::app().loadConfigFile(configPath);
  auto app_conf = drogon::app().getCustomConfig();

  LOG_INFO << app_conf["llama_model_file"].asString();
  // drogon::app().addListener("0.0.0.0", 8080);
  drogon::app().run();

  return 0;
}
