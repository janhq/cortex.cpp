#include "command_line_parser.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "commands/config_get_cmd.h"
#include "commands/config_upd_cmd.h"
#include "commands/cortex_upd_cmd.h"
#include "commands/engine_get_cmd.h"
#include "commands/engine_install_cmd.h"
#include "commands/engine_list_cmd.h"
#include "commands/engine_load_cmd.h"
#include "commands/engine_uninstall_cmd.h"
#include "commands/engine_unload_cmd.h"
#include "commands/engine_update_cmd.h"
#include "commands/engine_use_cmd.h"
#include "commands/hardware_activate_cmd.h"
#include "commands/model_del_cmd.h"
#include "commands/model_get_cmd.h"
#include "commands/model_import_cmd.h"
#include "commands/model_list_cmd.h"
#include "commands/model_pull_cmd.h"
#include "commands/model_source_add_cmd.h"
#include "commands/model_source_del_cmd.h"
#include "commands/model_source_list_cmd.h"
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
#include "utils/task_queue.h"

namespace {
constexpr const auto kCommonCommandsGroup = "Common Commands";
constexpr const auto kInferenceGroup = "Inference";
constexpr const auto kModelsGroup = "Models";
constexpr const auto kEngineGroup = "Engines";
constexpr const auto kHardwareGroup = "Hardware";
constexpr const auto kSystemGroup = "Server";
constexpr const auto kConfigGroup = "Configurations";
constexpr const auto kSubcommands = "Subcommands";
}  // namespace

CommandLineParser::CommandLineParser()
    : app_("\nCortex.cpp CLI\n"),
      download_service_{std::make_shared<DownloadService>()},
      dylib_path_manager_{std::make_shared<cortex::DylibPathManager>()},
      db_service_{std::make_shared<DatabaseService>()},
      engine_service_{std::make_shared<EngineService>(
          download_service_, dylib_path_manager_, db_service_,
          std::make_shared<cortex::TaskQueue>(1, "q"))} {}

bool CommandLineParser::SetupCommand(int argc, char** argv) {
  app_.usage("Usage:\n" + commands::GetCortexBinary() +
             " [options] [subcommand]");
  cml_data_.config = file_manager_utils::GetCortexConfig();
  std::string model_id;
  std::string msg;

  SetupCommonCommands();

  SetupModelCommands();

  SetupEngineCommands();

  SetupHardwareCommands();

  SetupSystemCommands();

  SetupConfigsCommands();

  app_.add_flag("--verbose", log_verbose, "Get verbose logs");

  // Logic is handled in main.cc, just for cli helper command
  std::string path;
  app_.add_option("--config_file_path", path, "Configure .rc file path");
  app_.add_option("--data_folder_path", path, "Configure data folder path");

  // cortex version
  auto cb = [&](int c) {
#ifdef CORTEX_CPP_VERSION
    CLI_LOG(CORTEX_CPP_VERSION);
#else
    CLI_LOG("default");
#endif
  };
  app_.add_flag_function("-v,--version", cb, "Get Cortex version");

  CLI11_PARSE(app_, argc, argv);
  if (argc == 1) {
    CLI_LOG(app_.help());
    return true;
  }

  // Check new update
#ifdef CORTEX_CPP_VERSION
  if (cml_data_.check_upd &&
      strcmp(CORTEX_CPP_VERSION, "default_version") != 0) {
    // TODO(sang) find a better way to handle
    // This is an extremely ugly way to deal with connection
    // hang when network down
    std::atomic<bool> done = false;
    std::thread t([&]() {
      if (auto latest_version =
              commands::CheckNewUpdate(commands::kTimeoutCheckUpdate);
          latest_version.has_value() && *latest_version != CORTEX_CPP_VERSION) {
        CLI_LOG("\nNew Cortex release available: "
                << CORTEX_CPP_VERSION << " -> " << *latest_version);
        CLI_LOG("To update, run: " << commands::GetRole()
                                   << commands::GetCortexBinary() << " update");
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
  // auto config = file_manager_utils::GetCortexConfig();
  // if (!config.llamacppVersion.empty() &&
  //     config.latestLlamacppRelease != config.llamacppVersion) {
  //   CLI_LOG(
  //       "\nNew llama.cpp version available: " << config.latestLlamacppRelease);
  //   CLI_LOG("To update, run: " << commands::GetCortexBinary()
  //                              << " engines update llama-cpp");
  // }

  return true;
}

void CommandLineParser::SetupCommonCommands() {
  auto model_pull_cmd = app_.add_subcommand(
      "pull",
      "Download models by HuggingFace Repo/ModelID"
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
      commands::ModelPullCmd().Exec(cml_data_.config.apiServerHost,
                                    std::stoi(cml_data_.config.apiServerPort),
                                    cml_data_.model_id);
    } catch (const std::exception& e) {
      CLI_LOG(e.what());
    }
  });

  auto run_cmd =
      app_.add_subcommand("run", "Shortcut: pull, start & chat with a model");
  run_cmd->group(kCommonCommandsGroup);
  run_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                 " run [options] [model_id]");
  run_cmd->add_option("model_id", cml_data_.model_id, "");
  run_cmd->add_option("--gpus", run_settings_["gpus"],
                      "List of GPU to activate, for example [0, 1]");
  run_cmd->add_option("--ctx_len", run_settings_["ctx_len"],
                      "Maximum context length for inference");
  run_cmd->add_flag("-d,--detach", cml_data_.run_detach, "Detached mode");
  run_cmd->callback([this, run_cmd] {
    if (std::exchange(executed_, true))
      return;
    commands::RunCmd rc(cml_data_.config.apiServerHost,
                        std::stoi(cml_data_.config.apiServerPort),
                        cml_data_.model_id, db_service_, engine_service_);
    rc.Exec(cml_data_.run_detach, run_settings_);
  });
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
  model_start_cmd->add_option("--gpus", run_settings_["gpus"],
                              "List of GPU to activate, for example [0, 1]");
  model_start_cmd->add_option("--ctx_len", run_settings_["ctx_len"],
                              "Maximum context length for inference");
  model_start_cmd->group(kSubcommands);
  model_start_cmd->callback([this, model_start_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_id.empty()) {
      CLI_LOG("[model_id] is required\n");
      CLI_LOG(model_start_cmd->help());
      return;
    };
    commands::ModelStartCmd(db_service_)
        .Exec(cml_data_.config.apiServerHost,
              std::stoi(cml_data_.config.apiServerPort), cml_data_.model_id,
              run_settings_);
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
    commands::ModelStopCmd().Exec(cml_data_.config.apiServerHost,
                                  std::stoi(cml_data_.config.apiServerPort),
                                  cml_data_.model_id);
  });

  auto list_models_cmd =
      models_cmd->add_subcommand("list", "List all local models");
  list_models_cmd->add_option("filter", cml_data_.filter, "Filter model id");
  list_models_cmd->add_flag("-e,--engine", cml_data_.display_engine,
                            "Display engine");
  list_models_cmd->add_flag("-v,--version", cml_data_.display_version,
                            "Display version");
  list_models_cmd->add_flag("--cpu_mode", cml_data_.display_cpu_mode,
                            "Display cpu mode");
  list_models_cmd->add_flag("--gpu_mode", cml_data_.display_gpu_mode,
                            "Display gpu mode");
  list_models_cmd->add_flag("--available", cml_data_.display_available_model,
                            "Display available models to download");
  list_models_cmd->group(kSubcommands);
  list_models_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    commands::ModelListCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), cml_data_.filter,
        cml_data_.display_engine, cml_data_.display_version,
        cml_data_.display_cpu_mode, cml_data_.display_gpu_mode,
        cml_data_.display_available_model);
  });

  auto get_models_cmd =
      models_cmd->add_subcommand("get", "Get a local model info by ID");
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
    commands::ModelGetCmd().Exec(cml_data_.config.apiServerHost,
                                 std::stoi(cml_data_.config.apiServerPort),
                                 cml_data_.model_id);
  });

  auto model_del_cmd =
      models_cmd->add_subcommand("delete", "Delete a local model by ID");
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

    commands::ModelDelCmd().Exec(cml_data_.config.apiServerHost,
                                 std::stoi(cml_data_.config.apiServerPort),
                                 cml_data_.model_id);
  });

  // Model update parameters comment
  ModelUpdate(models_cmd);

  std::string model_path;
  auto model_import_cmd = models_cmd->add_subcommand(
      "import", "Import a model from a local filepath");
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
    commands::ModelImportCmd().Exec(cml_data_.config.apiServerHost,
                                    std::stoi(cml_data_.config.apiServerPort),
                                    cml_data_.model_id, cml_data_.model_path);
  });

  auto model_source_cmd = models_cmd->add_subcommand(
      "sources", "Subcommands for managing model sources");
  model_source_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                          " models sources [options] [subcommand]");
  model_source_cmd->group(kSubcommands);

  model_source_cmd->callback([this, model_source_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (model_source_cmd->get_subcommands().empty()) {
      CLI_LOG(model_source_cmd->help());
    }
  });

  auto model_src_add_cmd =
      model_source_cmd->add_subcommand("add", "Add a model source");
  model_src_add_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                           " models sources add [model_source]");
  model_src_add_cmd->group(kSubcommands);
  model_src_add_cmd->add_option("source", cml_data_.model_src, "");
  model_src_add_cmd->callback([&]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_src.empty()) {
      CLI_LOG("[model_source] is required\n");
      CLI_LOG(model_src_add_cmd->help());
      return;
    };

    commands::ModelSourceAddCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), cml_data_.model_src);
  });

  auto model_src_del_cmd =
      model_source_cmd->add_subcommand("remove", "Remove a model source");
  model_src_del_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                           " models sources remove [model_source]");
  model_src_del_cmd->group(kSubcommands);
  model_src_del_cmd->add_option("source", cml_data_.model_src, "");
  model_src_del_cmd->callback([&]() {
    if (std::exchange(executed_, true))
      return;
    if (cml_data_.model_src.empty()) {
      CLI_LOG("[model_source] is required\n");
      CLI_LOG(model_src_del_cmd->help());
      return;
    };

    commands::ModelSourceDelCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), cml_data_.model_src);
  });

  auto model_src_list_cmd =
      model_source_cmd->add_subcommand("list", "List all model sources");
  model_src_list_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                            " models sources list");
  model_src_list_cmd->group(kSubcommands);
  model_src_list_cmd->callback([&]() {
    if (std::exchange(executed_, true))
      return;

    commands::ModelSourceListCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort));
  });
}

void CommandLineParser::SetupConfigsCommands() {
  auto config_cmd =
      app_.add_subcommand("config", "Subcommands for managing configurations");
  config_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                    " config [option] [value]");
  config_cmd->group(kConfigGroup);
  auto config_status_cmd =
      config_cmd->add_subcommand("status", "Print all configurations");
  config_status_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;
    commands::ConfigGetCmd().Exec(cml_data_.config.apiServerHost,
                                  std::stoi(cml_data_.config.apiServerPort));
  });

  for (const auto& [key, opt] : CONFIGURATIONS) {
    std::string option = "--" + opt.name;
    auto option_cmd =
        config_cmd->add_option(option, config_update_opts_[opt.name], opt.desc)
            ->group(opt.group)
            ->default_str(opt.default_value);

    if (opt.allow_empty) {
      option_cmd->expected(0, 1);
    } else {
      option_cmd->expected(1);
    }
  }

  config_cmd->callback([this, config_cmd] {
    if (std::exchange(executed_, true))
      return;

    auto is_empty = true;
    for (const auto& [key, value] : config_update_opts_) {
      if (!value.empty() || CONFIGURATIONS.at(key).allow_empty) {
        is_empty = false;
        break;
      }
    }
    if (is_empty) {
      CLI_LOG(config_cmd->help());
      return;
    }
    commands::ConfigUpdCmd().Exec(cml_data_.config.apiServerHost,
                                  std::stoi(cml_data_.config.apiServerPort),
                                  config_update_opts_);
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
    auto command = commands::EngineListCmd(engine_service_);
    command.Exec(cml_data_.config.apiServerHost,
                 std::stoi(cml_data_.config.apiServerPort));
  });

  auto install_cmd = engines_cmd->add_subcommand("install", "Install engine");
  install_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                     " engines install [engine_name] [options]");
  install_cmd->group(kSubcommands);
  install_cmd
      ->add_option("name", cml_data_.engine_name, "Engine name e.g. llama-cpp")
      ->required();
  install_cmd->add_option("-v, --version", cml_data_.engine_version,
                          "Engine version to download");
  install_cmd->add_option("-s, --source", cml_data_.engine_src,
                          "Install engine by local path");
  install_cmd->add_flag("-m, --menu", cml_data_.show_menu,
                        "Display menu for engine variant selection");

  install_cmd->callback([this, install_cmd] {
    if (std::exchange(executed_, true))
      return;
    try {
      commands::EngineInstallCmd(
          engine_service_, cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), cml_data_.show_menu)
          .Exec(cml_data_.engine_name, cml_data_.engine_version,
                cml_data_.engine_src);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });

  auto uninstall_cmd =
      engines_cmd->add_subcommand("uninstall", "Uninstall engine");
  uninstall_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                       " engines uninstall [engine_name] [options]");
  uninstall_cmd->group(kSubcommands);
  uninstall_cmd
      ->add_option("name", cml_data_.engine_name, "Engine name e.g. llama-cpp")
      ->required();
  uninstall_cmd->callback([this, uninstall_cmd] {
    if (std::exchange(executed_, true))
      return;
    try {
      commands::EngineUninstallCmd().Exec(
          cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), cml_data_.engine_name);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });

  auto engine_upd_cmd = engines_cmd->add_subcommand("update", "Update engine");
  engine_upd_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " engines update [engine_name]");
  engine_upd_cmd->group(kSubcommands);
  engine_upd_cmd
      ->add_option("name", cml_data_.engine_name, "Engine name e.g. llama-cpp")
      ->required();
  engine_upd_cmd->callback([this, engine_upd_cmd] {
    if (std::exchange(executed_, true))
      return;
    try {
      commands::EngineUpdateCmd().Exec(
          cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), cml_data_.engine_name);
    } catch (const std::exception& e) {
      CTL_ERR(e.what());
    }
  });

  auto engine_use_cmd =
      engines_cmd->add_subcommand("use", "Set engine as default");
  engine_use_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " engines use [engine_name]");
  engine_use_cmd->group(kSubcommands);
  engine_use_cmd
      ->add_option("name", cml_data_.engine_name, "Engine name e.g. llama-cpp")
      ->required();
  engine_use_cmd->callback([this, engine_use_cmd] {
    if (std::exchange(executed_, true))
      return;
    auto result = commands::EngineUseCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), cml_data_.engine_name);
    if (result.has_error()) {
      CTL_ERR(result.error());
    } else {
      CTL_INF("Engine " << cml_data_.engine_name << " is set as default");
    }
  });

  auto engine_load_cmd = engines_cmd->add_subcommand("load", "Load engine");
  engine_load_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                         " engines load [engine_name]");
  engine_load_cmd->group(kSubcommands);
  engine_load_cmd
      ->add_option("name", cml_data_.engine_name, "Engine name e.g. llama-cpp")
      ->required();
  engine_load_cmd->callback([this, engine_load_cmd] {
    if (std::exchange(executed_, true))
      return;
    auto result = commands::EngineLoadCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), cml_data_.engine_name);
    if (result.has_error()) {
      CTL_ERR(result.error());
    }
  });

  auto engine_unload_cmd =
      engines_cmd->add_subcommand("unload", "Unload engine");
  engine_unload_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                           " engines unload [engine_name]");
  engine_unload_cmd->group(kSubcommands);
  engine_unload_cmd
      ->add_option("name", cml_data_.engine_name, "Engine name e.g. llama-cpp")
      ->required();
  engine_unload_cmd->callback([this, engine_unload_cmd] {
    if (std::exchange(executed_, true))
      return;
    auto result = commands::EngineUnloadCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), cml_data_.engine_name);
    if (result.has_error()) {
      CTL_ERR(result.error());
    }
  });

  auto engine_get_cmd = engines_cmd->add_subcommand("get", "Get engine info");
  engine_get_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                        " engines get [engine_name] [options]");
  engine_get_cmd->group(kSubcommands);
  engine_get_cmd
      ->add_option("name", cml_data_.engine_name, "Engine name e.g. llama-cpp")
      ->required();
  engine_get_cmd->callback([this, engine_get_cmd] {
    if (std::exchange(executed_, true))
      return;
    commands::EngineGetCmd().Exec(cml_data_.config.apiServerHost,
                                  std::stoi(cml_data_.config.apiServerPort),
                                  cml_data_.engine_name);
  });
}

void CommandLineParser::SetupHardwareCommands() {
  // Hardware group commands
  auto hw_cmd =
      app_.add_subcommand("hardware", "Subcommands for managing hardware");
  hw_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                " hardware [options] [subcommand]");
  hw_cmd->group(kHardwareGroup);

  hw_cmd->callback([this, hw_cmd] {
    if (std::exchange(executed_, true))
      return;
    if (hw_cmd->get_subcommands().empty()) {
      CLI_LOG(hw_cmd->help());
    }
  });

  auto hw_list_cmd =
      hw_cmd->add_subcommand("list", "List all hardware information");

  hw_list_cmd->add_flag("--cpu", hw_opts_.show_cpu, "Display CPU information");
  hw_list_cmd->add_flag("--os", hw_opts_.show_os, "Display OS information");
  hw_list_cmd->add_flag("--ram", hw_opts_.show_ram, "Display RAM information");
  hw_list_cmd->add_flag("--storage", hw_opts_.show_storage,
                        "Display Storage information");
  hw_list_cmd->add_flag("--gpu", hw_opts_.show_gpu, "Display GPU information");
  hw_list_cmd->add_flag("--power", hw_opts_.show_power,
                        "Display Power information");
  hw_list_cmd->add_flag("--monitors", hw_opts_.show_monitors,
                        "Display Monitors information");

  hw_list_cmd->group(kSubcommands);
  hw_list_cmd->callback([this]() {
    if (std::exchange(executed_, true))
      return;
    if (hw_opts_.has_flag()) {
      commands::HardwareListCmd().Exec(
          cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), hw_opts_);
    } else {
      commands::HardwareListCmd().Exec(
          cml_data_.config.apiServerHost,
          std::stoi(cml_data_.config.apiServerPort), std::nullopt);
    }
  });

  auto hw_activate_cmd =
      hw_cmd->add_subcommand("activate", "Activate hardware");
  hw_activate_cmd->usage("Usage:\n" + commands::GetCortexBinary() +
                         " hardware activate --gpus [list_gpu]");
  hw_activate_cmd->group(kSubcommands);
  hw_activate_cmd->add_option("--gpus", run_settings_["gpus"],
                              "List of GPU to activate, for example [0, 1]");
  hw_activate_cmd->callback([this, hw_activate_cmd]() {
    if (std::exchange(executed_, true))
      return;
    if (hw_activate_cmd->get_options().empty()) {
      CLI_LOG(hw_activate_cmd->help());
      return;
    }

    if (run_settings_["gpus"].empty()) {
      CLI_LOG("[list_gpu] is required\n");
      CLI_LOG(hw_activate_cmd->help());
      return;
    }
    commands::HardwareActivateCmd().Exec(
        cml_data_.config.apiServerHost,
        std::stoi(cml_data_.config.apiServerPort), run_settings_);
  });
}

void CommandLineParser::SetupSystemCommands() {
  auto start_cmd = app_.add_subcommand("start", "Start the API server");
  start_cmd->group(kSystemGroup);

  // Add options dynamically
  std::vector<std::pair<std::string, std::string>> option_names = {
      {"logspath", "The directory where logs are stored"},
      {"logsllama", "The directory where llama-cpp engine logs are stored"},
      {"logsonnx", "The directory where onnx engine logs are stored"},
      {"datapath", "The directory for storing data"},
      {"loglines", "Log size limit"},
      {"host", "The host IP for the API server"},
      {"port", "The port used by the API server"},
      {"hf-token", "HuggingFace authentication token"},
      {"gh-agent", "Github user agent"},
      {"gh-token", "Github authentication token"},
      {"cors", "Cross-Origin Resource Sharing"},
      {"origins", "Lists allowed origins for CORS requests"},
      {"proxy-url", "Proxy URL"},
      {"verify-proxy", "SSL verification for client proxy connections"},
      {"verify-proxy-host", "SSL verification for host proxy connections"},
      {"proxy-username", "Proxy username"},
      {"proxy-password", "Proxy password"},
      {"no-proxy", "Specifies exceptions for proxy usage"},
      {"verify-ssl-peer", "SSL/TLS verification for peer connections"},
      {"verify-ssl-host", "SSL/TLS verification for host connections"},
      {"ssl-cert-path", "Path to SSL certificates"},
      {"ssl-key-path", "Path to SSL and keys"},
      {"loglevel", "Log level"}};
  cml_data_.server_start_options["loglevel"] = "INFO";
  for (const auto& option_name : option_names) {
    start_cmd->add_option(
        "--" + std::get<0>(option_name),
        cml_data_.server_start_options[std::get<0>(option_name)],
        std::get<1>(option_name));
  }

  start_cmd->callback([this] {
    if (std::exchange(executed_, true))
      return;

    commands::ServerStartCmd ssc;
    ssc.Exec(cml_data_.server_start_options["loglevel"],
             cml_data_.server_start_options, cml_data_.config);
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

  auto ps_cmd = app_.add_subcommand("ps", "Show active model statuses");
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
    auto cuc = commands::CortexUpdCmd(download_service_);
    cuc.Exec(cml_data_.cortex_version);
    cml_data_.check_upd = false;
  });
}

void CommandLineParser::ModelUpdate(CLI::App* parent) {
  auto model_update_cmd =
      parent->add_subcommand("update", "Update model configurations");
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
                                           "n_parallel",
                                           "cpu_threads",
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
    command.Exec(cml_data_.config.apiServerHost,
                 std::stoi(cml_data_.config.apiServerPort),
                 cml_data_.model_update_options);
  });
}
