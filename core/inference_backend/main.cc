#include <drogon/drogon.h>
#include <inference_lib.h>

int main()
{
    inference_aws::SDKOptions options;
    inference_aws::InitAPI(options);
    {

        drogon::app().loadConfigFile("/workspace/workdir/inference_backend/config.yaml");
        auto app_config = drogon::app().getCustomConfig();
        int drogon_port = app_config["drogon_port"].asInt();
        
        LOG_INFO << "Port of the server: " << drogon_port;
        drogon::app().addListener("0.0.0.0", drogon_port);
        unsigned int numCores = std::thread::hardware_concurrency();
        
        LOG_INFO << "Number of cores to be deployed (also threads num) :" << numCores;
        drogon::app().setThreadNum(numCores);
        drogon::app().run();
    }
    inference_aws::ShutdownAPI(options);
    return 0;
}
