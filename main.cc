#include <drogon/drogon.h>
#include <trantor/utils/logger.h>

int main()
{
    // Set HTTP listener address and port
    drogon::app().loadConfigFile("config/config.json");
    auto app_conf = drogon::app().getCustomConfig();

    LOG_INFO << app_conf["llama_model_file"].asString();
    drogon::app().addListener("0.0.0.0", 8080);
    drogon::app().run();
    return 0;
}
