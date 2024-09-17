#include "command_line_parser.h"
#include "commands/chat_cmd.h"
#include "commands/cmd_info.h"
#include "commands/cortex_upd_cmd.h"
#include "commands/engine_get_cmd.h"
#include "commands/engine_install_cmd.h"
#include "commands/engine_list_cmd.h"
#include "commands/engine_uninstall_cmd.h"
#include "commands/model_del_cmd.h"
#include "commands/model_get_cmd.h"
#include "commands/model_list_cmd.h"
#include "commands/model_pull_cmd.h"
#include "commands/model_start_cmd.h"
#include "commands/model_stop_cmd.h"
#include "commands/run_cmd.h"
#include "commands/server_start_cmd.h"
#include "commands/server_stop_cmd.h"
#include "config/yaml_config.h"
#include "services/engine_service.h"
#include "utils/file_manager_utils.h"
#include "utils/logging_utils.h"

CommandLineParser::CommandLineParser()
    : app_("Cortex.cpp CLI"), engine_service_{EngineService()} {}

bool CommandLineParser::SetupCommand(int argc, char** argv) {
  auto config = file_manager_utils::GetCortexConfig();
  std::string model_id;

  // Models group commands
  {
    auto models_cmd =
        app_.add_subcommand("models", "Subcommands for managing models");

    auto start_cmd = models_cmd->add_subcommand("start", "Start a model by ID");
    start_cmd->add_option("model_id", model_id, "");
    start_cmd->callback([&model_id, &config]() {
      commands::CmdInfo ci(model_id);
      std::string model_file =
          ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
      config::YamlHandler yaml_handler;
      yaml_handler.ModelConfigFromFile(
          file_manager_utils::GetModelsContainerPath().string() + "/" +
          model_file + ".yaml");
      commands::ModelStartCmd msc(config.apiServerHost,
                                  std::stoi(config.apiServerPort),
                                  yaml_handler.GetModelConfig());
      msc.Exec();
    });

    auto stop_model_cmd =
        models_cmd->add_subcommand("stop", "Stop a model by ID");
    stop_model_cmd->add_option("model_id", model_id, "");
    stop_model_cmd->callback([&model_id, &config]() {
      commands::CmdInfo ci(model_id);
      std::string model_file =
          ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
      config::YamlHandler yaml_handler;
      yaml_handler.ModelConfigFromFile(
          file_manager_utils::GetModelsContainerPath().string() + "/" +
          model_file + ".yaml");
      commands::ModelStopCmd smc(config.apiServerHost,
                                 std::stoi(config.apiServerPort),
                                 yaml_handler.GetModelConfig());
      smc.Exec();
    });

    auto list_models_cmd =
        models_cmd->add_subcommand("list", "List all models locally");
    list_models_cmd->callback([]() { commands::ModelListCmd().Exec(); });

    auto get_models_cmd =
        models_cmd->add_subcommand("get", "Get info of {model_id} locally");
    get_models_cmd->add_option("model_id", model_id, "");
    get_models_cmd->callback([&model_id]() {
      commands::ModelGetCmd command(model_id);
      command.Exec();
    });

    {
      auto model_pull_cmd =
          app_.add_subcommand("pull",
                              "Download a model from a registry. Working with "
                              "HuggingFace repositories. For available models, "
                              "please visit https://huggingface.co/cortexso");
      model_pull_cmd->add_option("model_id", model_id, "");
      model_pull_cmd->callback([&model_id]() {
        try {
          commands::ModelPullCmd().Exec(model_id);
        } catch (const std::exception& e) {
          CLI_LOG(e.what());
        }
      });
    }

    auto model_del_cmd =
        models_cmd->add_subcommand("delete", "Delete a model by ID locally");
    model_del_cmd->add_option("model_id", model_id, "");

    model_del_cmd->callback([&model_id]() {
      commands::ModelDelCmd mdc;
      mdc.Exec(model_id);
    });

    auto update_cmd =
        models_cmd->add_subcommand("update", "Update configuration of a model");
  }

  std::string msg;
  {
    auto chat_cmd =
        app_.add_subcommand("chat", "Send a chat request to a model");

    chat_cmd->add_option("model_id", model_id, "");
    chat_cmd->add_option("-m,--message", msg, "Message to chat with model");

    chat_cmd->callback([&model_id, &msg, &config] {
      commands::CmdInfo ci(model_id);
      std::string model_file =
          ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
      config::YamlHandler yaml_handler;
      yaml_handler.ModelConfigFromFile(
          file_manager_utils::GetModelsContainerPath().string() + "/" +
          model_file + ".yaml");
      commands::ChatCmd cc(config.apiServerHost,
                           std::stoi(config.apiServerPort),
                           yaml_handler.GetModelConfig());
      cc.Exec(msg);
    });
  }

  auto ps_cmd =
      app_.add_subcommand("ps", "Show running models and their status");

  auto embeddings_cmd = app_.add_subcommand(
      "embeddings", "Creates an embedding vector representing the input text");

  // Default version is latest
  std::string version{"latest"};
  {  // engines group commands
    auto engines_cmd = app_.add_subcommand("engines", "Get cortex engines");
    auto list_engines_cmd =
        engines_cmd->add_subcommand("list", "List all cortex engines");
    list_engines_cmd->callback([]() {
      commands::EngineListCmd command;
      command.Exec();
    });

    auto install_cmd = engines_cmd->add_subcommand("install", "Install engine");
    for (auto& engine : engine_service_.kSupportEngines) {
      std::string engine_name{engine};
      EngineInstall(install_cmd, engine_name, version);
    }

    auto uninstall_cmd =
        engines_cmd->add_subcommand("uninstall", "Uninstall engine");
    for (auto& engine : engine_service_.kSupportEngines) {
      std::string engine_name{engine};
      EngineUninstall(uninstall_cmd, engine_name);
    }

    EngineGet(engines_cmd);
  }

  {
    // cortex run tinyllama:gguf
    auto run_cmd =
        app_.add_subcommand("run", "Shortcut to start a model and chat");
    std::string model_id;
    run_cmd->add_option("model_id", model_id, "");
    run_cmd->callback([&model_id, &config] {
      commands::RunCmd rc(config.apiServerHost, std::stoi(config.apiServerPort),
                          model_id);
      rc.Exec();
    });
  }

  auto start_cmd = app_.add_subcommand("start", "Start the API server");
  int port = std::stoi(config.apiServerPort);
  start_cmd->add_option("-p, --port", port, "Server port to listen");
  start_cmd->callback([&config, &port] {
    if (port != stoi(config.apiServerPort)) {
      CTL_INF("apiServerPort changed from " << config.apiServerPort << " to "
                                            << port);
      auto config_path = file_manager_utils::GetConfigurationPath();
      config.apiServerPort = std::to_string(port);
      config_yaml_utils::DumpYamlConfig(config, config_path.string());
    }
    commands::ServerStartCmd ssc(config.apiServerHost, port);
    ssc.Exec();
  });

  auto stop_cmd = app_.add_subcommand("stop", "Stop the API server");

  stop_cmd->callback([&config] {
    commands::ServerStopCmd ssc(config.apiServerHost,
                                std::stoi(config.apiServerPort));
    ssc.Exec();
  });

  app_.add_flag("--verbose", log_verbose, "Verbose logging");

  // cortex version
  auto cb = [&](int c) {
#ifdef CORTEX_CPP_VERSION
    CLI_LOG(CORTEX_CPP_VERSION);
#else
    CLI_LOG("default");
#endif
  };
  app_.add_flag_function("-v,--version", cb, "Cortex version");

  std::string cortex_version;
  bool check_update = true;
  {
    auto update_cmd = app_.add_subcommand("update", "Update cortex version");

    update_cmd->add_option("-v", cortex_version, "");
    update_cmd->callback([&cortex_version, &check_update] {
      commands::CortexUpdCmd cuc;
      cuc.Exec(cortex_version);
      check_update = false;
    });
  }

  CLI11_PARSE(app_, argc, argv);
  if (argc == 1) {
    CLI_LOG(app_.help());
    return true;
  }

  // Check new update, only check for stable release for now
#ifdef CORTEX_CPP_VERSION
  if (check_update) {
    commands::CheckNewUpdate();
  }
#endif

  return true;
}

void CommandLineParser::EngineInstall(CLI::App* parent,
                                      const std::string& engine_name,
                                      std::string& version) {
  auto install_engine_cmd = parent->add_subcommand(engine_name, "");

  install_engine_cmd->callback([engine_name, version] {
    try {
      commands::EngineInstallCmd().Exec(engine_name, version);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });
}

void CommandLineParser::EngineUninstall(CLI::App* parent,
                                        const std::string& engine_name) {
  auto uninstall_engine_cmd = parent->add_subcommand(engine_name, "");

  uninstall_engine_cmd->callback([engine_name] {
    try {
      commands::EngineUninstallCmd().Exec(engine_name);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });
}

void CommandLineParser::EngineGet(CLI::App* parent) {
  auto get_cmd = parent->add_subcommand("get", "Get an engine info");

  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    std::string desc = "Get " + engine_name + " status";

    auto engine_get_cmd = get_cmd->add_subcommand(engine_name, desc);
    engine_get_cmd->callback(
        [engine_name] { commands::EngineGetCmd().Exec(engine_name); });
  }
}
