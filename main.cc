
#include <drogon/drogon.h>
#include <libgen.h>  // for dirname()
#include <unistd.h>  // for readlink()
#include <climits>   // for PATH_MAX

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#endif

int main()
{
    char path[PATH_MAX];
    std::string configPath;

#if defined(__APPLE__) && defined(__MACH__)
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        path[size] = '\0';  // Null-terminate the string
        char *dir = dirname(path);
        configPath = std::string(dir) + "/config/config.json";
    } else {
        LOG_ERROR << "Failed to get binary location!";
        return 1;
    }
#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)-1);
    if (len != -1) {
        path[len] = '\0';
        char *dir = dirname(path);
        configPath = std::string(dir) + "/config/config.json";
    } else {
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
    //drogon::app().addListener("0.0.0.0", 8080);
    drogon::app().run();

    return 0;
}

