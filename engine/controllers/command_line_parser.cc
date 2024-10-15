#include "command_line_parser.h"
#include <memory>
#include "commands/chat_cmd.h"
#include "commands/chat_completion_cmd.h"
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
#include "commands/model_upd_cmd.h"
#include "commands/ps_cmd.h"
#include "commands/run_cmd.h"
#include "commands/server_start_cmd.h"
#include "commands/server_stop_cmd.h"
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
    : app_("Cortex.cpp CLI"),
      model_service_{ModelService(std::make_shared<DownloadService>())},
      engine_service_{EngineService(std::make_shared<DownloadService>())} {}

bool CommandLineParser::SetupCommand(int argc, char** argv) {
  app_.usage("Usage:\n" + commands::GetCortexBinary() +
             " [options] [subcommand]");
  cml_data_.config = file_manager_utils::GetCortexConfig();
  std::string model_id;
  std::string msg;

  SetupCommonCommands();

  SetupInferenceCommands();

  SetupModelCommands();

  SetupEngineCommands();

  SetupSystemCommands();

  app_.add_flag("--verbose", log_verbose, "Verbose logging");

  // Logic is handled in main.cc, just for cli helper command
  std::string path;
  app_.add_option("--config_file_path", path, "Configuration file path");
  app_.add_option("--data_folder_path", path, "Data folder path");

  // cortex version
  auto cb = [&](int c) {
#ifdef CORTEX_CPP_VERSION
    CLI_LOG(CORTEX_CPP_VERSION);
#else
    CLI_LOG("default");
#endif
  };
  app_.add_flag_function("-v,--version", cb, "Cortex version");

  CLI11_PARSE(app_, argc, argv);
  if (argc == 1) {
    CLI_LOG(app_.help());
    return true;
  }

  // Check new update
#ifdef CORTEX_CPP_VERSION
  if (cml_data_.check_upd) {
    // TODO(sang) find a better way to handle
    // This is an extremely ungly way to deal with connection
    // hang when network down
    std::atomic<bool> done = false;
    std::thread t([&]() {
      if (auto latest_version =
              commands::CheckNewUpdate(commands::kTimeoutCheckUpdate);
          latest_version.has_value() && *latest_version != CORTEX_CPP_VERSION) {
        CLI_LOG("\nA new release of cortex is available: "
                << CORTEX_CPP_VERSION << " -> " << *latest_version);
        CLI_LOG("To upgrade, run: " << commands::GetRole()
                                    << commands::GetCortexBinary()
                                    << " update");
      }
      done = true;
    });
    // Do not wait for http connection timeout
    t.detach();
    int retry = 10;
    while (!done && retry--) {
      std::this_thread::sleep_for(commands::kTimeoutCheckUpdate / 10);
    }
  }
#endif

  return true;
}

void CommandLineParser::SetupCommonCommands() {
  auto model_pull_cmd = app_.add_subcommand(
      "pull",
      "Download a model by URL (or HuggingFace ID) "
      "See built-in models: https://huggingface.co/cortexso");
  model_pull_cmd->group(kCommonCommandsGroup);
  model_pull_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " pull [options] [model_id]");
  model_pull_cmd->add_option("model_id", cml_data_.model_id, "");
  model_pull_cmd->callback([this, model_pull_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_pull_cmd->help());
      return;
    }
    try {
      commands::ModelPullCmd().Exec(cml_data_.model_id);
    } catch (const std::exception& e) {
      CLI_LOG(e.what());
    }
  });

  auto run_cmd = app_.add_subcommand("run", "Shortcut to start a model");
  run_cmd->group(kCommonCommandsGroup);
  run_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                 " run [options] [model_id]");
  run_cmd->add_option("model_id", cml_data_.model_id, "");
  run_cmd->add_flag("--chat", cml_data_.chat_flag, "Flag for interactive mode");
  run_cmd->callback([this, run_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(run_cmd->help());
      return;
    }
    commands::RunCmd rc(cml_data_.config.apiServerHost,
                        std::stoi(cml_data_.config.apiServerPort),
                        cml_data_.model_id);
    rc.Exec(cml_data_.chat_flag);
  });

  auto chat_cmd = app_.add_subcommand(
      "chat",
      "Shortcut for `cortex run --chat` or send a chat completion request");
  chat_cmd->group(kCommonCommandsGroup);
  chat_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                  " chat [model_id] -m [msg]");
  chat_cmd->add_option("model_id", cml_data_.model_id, "");
  chat_cmd->add_option("-m,--message", cml_data_.msg,
                       "Message to chat with model");
  chat_cmd->callback([this, chat_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(chat_cmd->help());
      return;
    }

    if (cml_data_.msg.empty()) {
      commands::ChatCmd().Exec(cml_data_.config.apiServerHost,
                               std::stoi(cml_data_.config.apiServerPort),
                               cml_data_.model_id);
    } else {
      commands::ChatCompletionCmd(model_service_)
          .Exec(cml_data_.config.apiServerHost,
                std::stoi(cml_data_.config.apiServerPort), cml_data_.model_id,
                cml_data_.msg);
    }
  });
}

void CommandLineParser::SetupInferenceCommands() {
  auto embeddings_cmd = app_.add_subcommand(
      "embeddings", "Creates an embedding vector representing the input text");
  embeddings_cmd->group(kInferenceGroup);
}

void CommandLineParser::SetupModelCommands() {
  // Models group commands
  auto models_cmd =
      app_.add_subcommand("models", "Subcommands for managing models");
  models_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                    " models [options] [subcommand]");
  models_cmd->group(kModelsGroup);

  models_cmd->callback([this, models_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (models_cmd->get_subcommands().empty()) {
      CLI_LOG(models_cmd->help());
    }
  });

  auto model_start_cmd =
      models_cmd->add_subcommand("start", "Start a model by ID");
  model_start_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                         " models start [model_id]");
  model_start_cmd->add_option("model_id", cml_data_.model_id, "");
  model_start_cmd->group(kSubcommands);
  model_start_cmd->callback([this, model_start_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_start_cmd->help());
      return;
    };
    commands::ModelStartCmd(model_service_)
        .Exec(cml_data_.config.apiServerHost,
              std::stoi(cml_data_.config.apiServerPort), cml_data_.model_id);
  });

  auto stop_model_cmd =
      models_cmd->add_subcommand("stop", "Stop a model by ID");
  stop_model_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " models stop [model_id]");
  stop_model_cmd->group(kSubcommands);
  stop_model_cmd->add_option("model_id", cml_data_.model_id, "");
  stop_model_cmd->callback([this, stop_model_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(stop_model_cmd->help());
      return;
    };
    commands::ModelStopCmd(model_service_)
        .Exec(cml_data_.config.apiServerHost,
              std::stoi(cml_data_.config.apiServerPort), cml_data_.model_id);
  });

  auto list_models_cmd =
      models_cmd->add_subcommand("list", "List all models locally");
  list_models_cmd->group(kSubcommands);
  list_models_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    commands::ModelListCmd().Exec();
  });

  auto get_models_cmd =
      models_cmd->add_subcommand("get", "Get info of {model_id} locally");
  get_models_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " models get [model_id]");
  get_models_cmd->group(kSubcommands);
  get_models_cmd->add_option("model_id", cml_data_.model_id, "");
  get_models_cmd->callback([this, get_models_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(get_models_cmd->help());
      return;
    };
    commands::ModelGetCmd().Exec(cml_data_.model_id);
  });

  auto model_del_cmd =
      models_cmd->add_subcommand("delete", "Delete a model by ID locally");
  model_del_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                       " models delete [model_id]");
  model_del_cmd->group(kSubcommands);
  model_del_cmd->add_option("model_id", cml_data_.model_id, "");
  model_del_cmd->callback([&]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_del_cmd->help());
      return;
    };
    commands::ModelDelCmd().Exec(cml_data_.model_id);
  });

  std::string model_alias;
  auto model_alias_cmd =
      models_cmd->add_subcommand("alias", "Add alias name for a modelID");
  model_alias_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                         " models alias --model_id [model_id] --alias [alias]");
  model_alias_cmd->group(kSubcommands);
  model_alias_cmd->add_option(
      "--model_id", cml_data_.model_id,
      "Can be modelID or model alias to identify model");
  model_alias_cmd->add_option("--alias", cml_data_.model_alias,
                              "new alias to be set");
  model_alias_cmd->callback([this, model_alias_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty() || cml_data_.model_alias.empty()) {
      CLI_LOG("[model_id] and [alias] are required\n");
      CLI_LOG(model_alias_cmd->help());
      return;
    }
    commands::ModelAliasCmd mdc;
    mdc.Exec(cml_data_.model_id, cml_data_.model_alias);
  });
  // Model update parameters comment
  ModelUpdate(models_cmd);

  std::string model_path;
  auto model_import_cmd = models_cmd->add_subcommand(
      "import", "Import a gguf model from local file");
  model_import_cmd->usage(
      "Usage:\n" + commands::GetCortexBinary() +
      " models import --model_id [model_id] --model_path [model_path]");
  model_import_cmd->group(kSubcommands);
  model_import_cmd->add_option("--model_id", cml_data_.model_id, "");
  model_import_cmd->add_option("--model_path", cml_data_.model_path,
                               "Absolute path to .gguf model, the path should "
                               "include the gguf file name");
  model_import_cmd->callback([this, model_import_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty() || cml_data_.model_path.empty()) {
      CLI_LOG("[model_id] and [model_path] are required\n");
      CLI_LOG(model_import_cmd->help());
      return;
    }
    commands::ModelImportCmd command(cml_data_.model_id, cml_data_.model_path);
    command.Exec();
  });
}

void CommandLineParser::SetupEngineCommands() {
  auto engines_cmd =
      app_.add_subcommand("engines", "Subcommands for managing engines");
  engines_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                     " engines [options] [subcommand]");
  engines_cmd->group(kEngineGroup);
  engines_cmd->callback([this, engines_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (engines_cmd->get_subcommands().empty()) {
      CLI_LOG("A subcommand is required\n");
      CLI_LOG(engines_cmd->help());
    }
  });

  auto list_engines_cmd =
      engines_cmd->add_subcommand("list", "List all cortex engines");
  list_engines_cmd->group(kSubcommands);
  list_engines_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    commands::EngineListCmd command;
    command.Exec();
  });

  auto install_cmd = engines_cmd->add_subcommand("install", "Install engine");
  install_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                     " engines install [engine_name] [options]");
  install_cmd->group(kSubcommands);
  install_cmd->callback([this, install_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (install_cmd->get_subcommands().empty()) {
      CLI_LOG("[engine_name] is required\n");
      CLI_LOG(install_cmd->help());
    }
  });
  for (auto& engine : engine_service_.kSupportEngines) {
    std::string engine_name{engine};
    EngineInstall(install_cmd, engine_name, cml_data_.engine_version,
                  cml_data_.engine_src);
  }

  auto uninstall_cmd =
      engines_cmd->add_subcommand("uninstall", "Uninstall engine");
  uninstall_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                       " engines uninstall [engine_name] [options]");
  uninstall_cmd->callback([this, uninstall_cmd] {
    if (std::exchange(executed_, true))
      return;
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
}

void CommandLineParser::SetupSystemCommands() {
  auto start_cmd = app_.add_subcommand("start", "Start the API server");
  start_cmd->group(kSystemGroup);
  cml_data_.port = std::stoi(cml_data_.config.apiServerPort);
  start_cmd->add_option("-p, --port", cml_data_.port, "Server port to listen");
  start_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.port != stoi(cml_data_.config.apiServerPort)) {
      CTL_INF("apiServerPort changed from " << cml_data_.config.apiServerPort
                                            << " to " << cml_data_.port);
      auto config_path = file_manager_utils::GetConfigurationPath();
      cml_data_.config.apiServerPort = std::to_string(cml_data_.port);
      config_yaml_utils::DumpYamlConfig(cml_data_.config, config_path.string());
    }
    commands::ServerStartCmd ssc;
    ssc.Exec(cml_data_.config.apiServerHost,
             std::stoi(cml_data_.config.apiServerPort));
  });

  auto stop_cmd = app_.add_subcommand("stop", "Stop the API server");
  stop_cmd->group(kSystemGroup);
  stop_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;
    commands::ServerStopCmd ssc(cml_data_.config.apiServerHost,
                                std::stoi(cml_data_.config.apiServerPort));
    ssc.Exec();
  });

  auto ps_cmd =
      app_.add_subcommand("ps", "Show running models and their status");
  ps_cmd->group(kSystemGroup);
  ps_cmd->usage("Usage:\n" + commands::GetCortexBinary() + "ps");
  ps_cmd->callback([&]() {
    if (std::exchange(executed_, true))
      return;
    commands::PsCmd().Exec(cml_data_.config.apiServerHost,
                           std::stoi(cml_data_.config.apiServerPort));
  });

  auto update_cmd = app_.add_subcommand("update", "Update cortex version");
  update_cmd->group(kSystemGroup);
  update_cmd->add_option("-v", cml_data_.cortex_version, "");
  update_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;
#if !defined(_WIN32)
    if (getuid()) {
      CLI_LOG("Error: Not root user. Please run with sudo.");
      return;
    }
#endif
    commands::CortexUpdCmd cuc;
    cuc.Exec(cml_data_.cortex_version);
    cml_data_.check_upd = false;
  });
}

void CommandLineParser::EngineInstall(CLI::App* parent,
                                      const std::string& engine_name,
                                      std::string& version, std::string& src) {
  auto install_engine_cmd = parent->add_subcommand(engine_name, "");
  install_engine_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                            " engines install " + engine_name + " [options]");
  install_engine_cmd->group(kEngineGroup);

  install_engine_cmd->add_option("-v, --version", version,
                                 "Engine version to download");

  install_engine_cmd->add_option("-s, --source", src,
                                 "Install engine by local path");

  install_engine_cmd->callback([this, engine_name, &version, &src] {
    if (std::exchange(executed_, true))
      return;
    try {
      commands::EngineInstallCmd().Exec(engine_name, version, src);
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

  uninstall_engine_cmd->callback([this, engine_name] {
    if (std::exchange(executed_, true))
      return;
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
  get_cmd->callback([this, get_cmd] {
    if (std::exchange(executed_, true))
      return;
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
    engine_get_cmd->callback([this, engine_name] {
      if (std::exchange(executed_, true))
        return;
      commands::EngineGetCmd().Exec(engine_name);
    });
  }
}

void CommandLineParser::ModelUpdate(CLI::App* parent) {
  auto model_update_cmd =
      parent->add_subcommand("update", "Update configuration of a model");
  model_update_cmd->group(kSubcommands);
  model_update_cmd->add_option("--model_id", cml_data_.model_id, "Model ID")
      ->required();

  // Add options dynamically
  std::vector<std::string> option_names = {"name",
                                           "model",
                                           "version",
                                           "stop",
                                           "top_p",
                                           "temperature",
                                           "frequency_penalty",
                                           "presence_penalty",
                                           "max_tokens",
                                           "stream",
                                           "ngl",
                                           "ctx_len",
                                           "engine",
                                           "prompt_template",
                                           "system_template",
                                           "user_template",
                                           "ai_template",
                                           "os",
                                           "gpu_arch",
                                           "quantization_method",
                                           "precision",
                                           "tp",
                                           "trtllm_version",
                                           "text_model",
                                           "files",
                                           "created",
                                           "object",
                                           "owned_by",
                                           "seed",
                                           "dynatemp_range",
                                           "dynatemp_exponent",
                                           "top_k",
                                           "min_p",
                                           "tfs_z",
                                           "typ_p",
                                           "repeat_last_n",
                                           "repeat_penalty",
                                           "mirostat",
                                           "mirostat_tau",
                                           "mirostat_eta",
                                           "penalize_nl",
                                           "ignore_eos",
                                           "n_probs",
                                           "min_keep",
                                           "grammar"};

  for (const auto& option_name : option_names) {
    model_update_cmd->add_option("--" + option_name,
                                 cml_data_.model_update_options[option_name],
                                 option_name);
  }

  model_update_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    commands::ModelUpdCmd command(cml_data_.model_id);
    command.Exec(cml_data_.model_update_options);
  });
}
