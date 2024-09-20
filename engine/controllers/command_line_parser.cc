#include "command_line_parser.h"
#include "commands/chat_cmd.h"
#include "commands/cmd_info.h"
#include "commands/cortex_upd_cmd.h"
#include "commands/engine_get_cmd.h"
#include "commands/engine_install_cmd.h"
#include "commands/engine_list_cmd.h"
#include "commands/engine_uninstall_cmd.h"
#include "commands/model_alias_cmd.h"
#include "commands/model_del_cmd.h"
#include "commands/model_get_cmd.h"
#include "commands/model_import_cmd.h"
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

namespace {
constexpr const auto kCommonCommandsGroup = "Common Commands";
constexpr const auto kInferenceGroup = "Inference";
constexpr const auto kModelsGroup = "Models";
constexpr const auto kEngineGroup = "Engines";
constexpr const auto kSystemGroup = "System";
constexpr const auto kSubcommands = "Subcommands";
}  // namespace
CommandLineParser::CommandLineParser()
    : app_("Cortex.cpp CLI"), engine_service_{EngineService()} {}

bool CommandLineParser::SetupCommand(int argc, char** argv) {
  app_.usage("Usage:\n" + commands::GetCortexBinary() +
             " [options] [subcommand]");
  auto config = file_manager_utils::GetCortexConfig();
  std::string model_id;
  std::string msg;

  auto model_pull_cmd = app_.add_subcommand(
      "pull",
      "Download a model by URL (or HuggingFace ID) "
      "See built-in models: https://huggingface.co/cortexso");
  model_pull_cmd->group(kCommonCommandsGroup);
  model_pull_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " pull [options] [model_id]");
  model_pull_cmd->add_option("model_id", model_id, "");
  model_pull_cmd->callback([model_pull_cmd, &model_id]() {
    if (model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_pull_cmd->help());
      return;
    }
    try {
      commands::ModelPullCmd().Exec(model_id);
    } catch (const std::exception& e) {
      CLI_LOG(e.what());
    }
  });

  auto run_cmd =
      app_.add_subcommand("run", "Shortcut to start a model and chat");
  run_cmd->group(kCommonCommandsGroup);
  run_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " run [options] [model_id]");
  run_cmd->add_option("model_id", model_id, "");
  run_cmd->callback([run_cmd, &model_id, &config] {
    if (model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(run_cmd->help());
      return;
    }
    commands::RunCmd rc(config.apiServerHost, std::stoi(config.apiServerPort),
                        model_id);
    rc.Exec();
  });

  auto chat_cmd = app_.add_subcommand("chat", "Send a chat completion request");
  chat_cmd->group(kCommonCommandsGroup);
  chat_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " chat [model_id] [options]");
  chat_cmd->add_option("model_id", model_id, "");
  chat_cmd->add_option("-m,--message", msg, "Message to chat with model");
  chat_cmd->callback([chat_cmd, &model_id, &msg, &config] {
    if (model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(chat_cmd->help());
      return;
    }
    commands::CmdInfo ci(model_id);
    std::string model_file =
        ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
    config::YamlHandler yaml_handler;
    yaml_handler.ModelConfigFromFile(
        file_manager_utils::GetModelsContainerPath().string() + "/" +
        model_file + ".yaml");
    commands::ChatCmd cc(config.apiServerHost, std::stoi(config.apiServerPort),
                         yaml_handler.GetModelConfig());
    cc.Exec(msg);
  });

  auto embeddings_cmd = app_.add_subcommand(
      "embeddings", "Creates an embedding vector representing the input text");
  embeddings_cmd->group(kInferenceGroup);

  // Models group commands
  auto models_cmd =
      app_.add_subcommand("models", "Subcommands for managing models");
  models_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                    " models [options] [subcommand]");
  models_cmd->group(kModelsGroup);

  models_cmd->callback([&] {
    if (models_cmd->get_subcommands().empty()) {
      CLI_LOG(models_cmd->help());
    }
  });

  auto model_start_cmd =
      models_cmd->add_subcommand("start", "Start a model by ID");
  model_start_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                         " models start [model_id]");
  model_start_cmd->add_option("model_id", model_id, "");
  model_start_cmd->group(kSubcommands);
  model_start_cmd->callback([&model_start_cmd, &model_id, &config]() {
    if (model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_start_cmd->help());
      return;
    };
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
  stop_model_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " models stop [model_id]");
  stop_model_cmd->group(kSubcommands);
  stop_model_cmd->add_option("model_id", model_id, "");
  stop_model_cmd->callback([&stop_model_cmd, &model_id, &config]() {
    if (model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(stop_model_cmd->help());
      return;
    };
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
  list_models_cmd->group(kSubcommands);
  list_models_cmd->callback([]() { commands::ModelListCmd().Exec(); });

  auto get_models_cmd =
      models_cmd->add_subcommand("get", "Get info of {model_id} locally");
  get_models_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " models get [model_id]");
  get_models_cmd->group(kSubcommands);
  get_models_cmd->add_option("model_id", model_id, "");
  get_models_cmd->callback([&get_models_cmd, &model_id]() {
    if (model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(get_models_cmd->help());
      return;
    };
    commands::ModelGetCmd().Exec(model_id);
  });

  auto model_del_cmd =
      models_cmd->add_subcommand("delete", "Delete a model by ID locally");
  model_del_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                       " models delete [model_id]");
  model_del_cmd->group(kSubcommands);
  model_del_cmd->add_option("model_id", model_id, "");
  model_del_cmd->callback([&model_del_cmd, &model_id]() {
    if (model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_del_cmd->help());
      return;
    };
    commands::ModelDelCmd mdc;
    mdc.Exec(model_id);
  });

  std::string model_alias;
  auto model_alias_cmd =
      models_cmd->add_subcommand("alias", "Add alias name for a modelID");
  model_alias_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                         " models alias --model_id [model_id] --alias [alias]");
  model_alias_cmd->group(kSubcommands);
  model_alias_cmd->add_option(
      "--model_id", model_id,
      "Can be modelID or model alias to identify model");
  model_alias_cmd->add_option("--alias", model_alias, "new alias to be set");
  model_alias_cmd->callback([&model_alias_cmd, &model_id, &model_alias]() {
    if (model_id.empty() || model_alias.empty()) {
      CLI_LOG("[model_id] and [alias] are required\n");
      CLI_LOG(model_alias_cmd->help());
      return;
    }
    commands::ModelAliasCmd mdc;
    mdc.Exec(model_id, model_alias);
  });

  auto model_update_cmd =
      models_cmd->add_subcommand("update", "Update configuration of a model");
  model_update_cmd->group(kSubcommands);

  std::string model_path;
  auto model_import_cmd = models_cmd->add_subcommand(
      "import", "Import a gguf model from local file");
  model_import_cmd->usage(
      "Usage:\n" + commands::GetCortexBinary() +
      " models import --model_id [model_id] --model_path [model_path]");
  model_import_cmd->group(kSubcommands);
  model_import_cmd->add_option("--model_id", model_id, "");
  model_import_cmd->add_option("--model_path", model_path,
                               "Absolute path to .gguf model, the path should "
                               "include the gguf file name");
  model_import_cmd->callback([&model_import_cmd, &model_id, &model_path]() {
    if (model_id.empty() || model_path.empty()) {
      CLI_LOG("[model_id] and [model_path] are required\n");
      CLI_LOG(model_import_cmd->help());
      return;
    }
    commands::ModelImportCmd command(model_id, model_path);
    command.Exec();
  });

  // Default version is latest
  std::string version{"latest"};
  // engines group commands
  auto engines_cmd =
      app_.add_subcommand("engines", "Subcommands for managing engines");
  engines_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                     " engines [options] [subcommand]");
  engines_cmd->group(kEngineGroup);
  engines_cmd->callback([&] {
    if (engines_cmd->get_subcommands().empty()) {
      CLI_LOG("A subcommand is required\n");
      CLI_LOG(engines_cmd->help());
    }
  });

  auto list_engines_cmd =
      engines_cmd->add_subcommand("list", "List all cortex engines");
  list_engines_cmd->group(kSubcommands);
  list_engines_cmd->callback([]() {
    commands::EngineListCmd command;
    command.Exec();
  });

  auto install_cmd = engines_cmd->add_subcommand("install", "Install engine");
  install_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                     " engines install [engine_name] [options]");
  install_cmd->group(kSubcommands);
  install_cmd->callback([&install_cmd] {
    if (install_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(install_cmd->help());
    }
  });
  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    EngineInstall(install_cmd, engine_name, version);
  }

  auto uninstall_cmd =
      engines_cmd->add_subcommand("uninstall", "Uninstall engine");
  uninstall_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                       " engines uninstall [engine_name] [options]");
  uninstall_cmd->callback([&uninstall_cmd] {
    if (uninstall_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(uninstall_cmd->help());
    }
  });
  uninstall_cmd->group(kSubcommands);
  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    EngineUninstall(uninstall_cmd, engine_name);
  }

  EngineGet(engines_cmd);

  auto start_cmd = app_.add_subcommand("start", "Start the API server");
  start_cmd->group(kSystemGroup);
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
    commands::ServerStartCmd ssc;
    ssc.Exec(config.apiServerHost, std::stoi(config.apiServerPort));
  });

  auto stop_cmd = app_.add_subcommand("stop", "Stop the API server");
  stop_cmd->group(kSystemGroup);
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

  auto update_cmd = app_.add_subcommand("update", "Update cortex version");
  update_cmd->group(kSystemGroup);
  update_cmd->add_option("-v", cortex_version, "");
  update_cmd->callback([&cortex_version, &check_update] {
    commands::CortexUpdCmd cuc;
    cuc.Exec(cortex_version);
    check_update = false;
  });

  auto ps_cmd =
      app_.add_subcommand("ps", "Show running models and their status");
  ps_cmd->group(kSystemGroup);

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
  install_engine_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                            " engines install " + engine_name + " [options]");
  install_engine_cmd->group(kEngineGroup);

  install_engine_cmd->add_option("-v, --version", version,
                                 "Engine version to download");

  install_engine_cmd->callback([engine_name, &version] {
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
  uninstall_engine_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                              " engines install " + engine_name + " [options]");
  uninstall_engine_cmd->group(kEngineGroup);

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
  get_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                 " engines get [engine_name] [options]");
  get_cmd->group(kSubcommands);
  get_cmd->callback([get_cmd] {
    if (get_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(get_cmd->help());
    }
  });

  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    std::string desc = "Get " + engine_name + " status";

    auto engine_get_cmd = get_cmd->add_subcommand(engine_name, desc);
    engine_get_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                          " engines get " + engine_name + " [options]");
    engine_get_cmd->group(kEngineGroup);
    engine_get_cmd->callback(
        [engine_name] { commands::EngineGetCmd().Exec(engine_name); });
  }
}
