

#include <drogon/drogon.h>
#include <trantor/utils/logger.h>
#include <mach-o/dyld.h>
#include <libgen.h>  // for dirname()

int main()
{
    char path[PATH_MAX];
    uint32_t size = sizeof(path);

    if (_NSGetExecutablePath(path, &size) == 0) {
        // Null-terminate the string
        path[size] = '\0';

        // Get directory of the binary
        char *dir = dirname(path);

        // Construct the path to the config file
        std::string configPath = std::string(dir) + "/config/config.json";

        // Set HTTP listener address and port
        drogon::app().loadConfigFile(configPath);
        auto app_conf = drogon::app().getCustomConfig();

        LOG_INFO << app_conf["llama_model_file"].asString();
        drogon::app().addListener("0.0.0.0", 8080);
        drogon::app().run();
    } else {
        LOG_ERROR << "Failed to get binary location!";
        return 1;
    }

    return 0;
}
