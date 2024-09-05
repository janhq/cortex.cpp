#include "command_line_parser.h"
#include "commands/chat_cmd.h"
#include "commands/cmd_info.h"
#include "commands/cortex_upd_cmd.h"
#include "commands/engine_init_cmd.h"
#include "commands/engine_list_cmd.h"
#include "commands/model_get_cmd.h"
#include "commands/model_list_cmd.h"
#include "commands/model_pull_cmd.h"
#include "commands/model_start_cmd.h"
#include "commands/model_stop_cmd.h"
#include "commands/run_cmd.h"
#include "commands/server_stop_cmd.h"
#include "config/yaml_config.h"
#include "httplib.h"
#include "utils/cortex_utils.h"
#include "utils/logging_utils.h"

CommandLineParser::CommandLineParser() : app_("Cortex.cpp CLI") {}

bool CommandLineParser::SetupCommand(int argc, char** argv) {

  std::string model_id;

  // Models group commands
  {
    auto models_cmd =
        app_.add_subcommand("models", "Subcommands for managing models");

    auto start_cmd = models_cmd->add_subcommand("start", "Start a model by ID");
    start_cmd->add_option("model_id", model_id, "");
    start_cmd->callback([&model_id]() {
      commands::CmdInfo ci(model_id);
      std::string model_file =
          ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
      config::YamlHandler yaml_handler;
      yaml_handler.ModelConfigFromFile(cortex_utils::GetCurrentPath() +
                                       "/models/" + model_file + ".yaml");
      commands::ModelStartCmd msc("127.0.0.1", 3928,
                                  yaml_handler.GetModelConfig());
      msc.Exec();
    });

    auto stop_model_cmd =
        models_cmd->add_subcommand("stop", "Stop a model by ID");
    stop_model_cmd->add_option("model_id", model_id, "");
    stop_model_cmd->callback([&model_id]() {
      commands::CmdInfo ci(model_id);
      std::string model_file =
          ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
      config::YamlHandler yaml_handler;
      yaml_handler.ModelConfigFromFile(cortex_utils::GetCurrentPath() +
                                       "/models/" + model_file + ".yaml");
      commands::ModelStopCmd smc("127.0.0.1", 3928,
                                 yaml_handler.GetModelConfig());
      smc.Exec();
    });

    auto list_models_cmd =
        models_cmd->add_subcommand("list", "List all models locally");
    list_models_cmd->callback([]() {
      commands::ModelListCmd command;
      command.Exec();
    });

    auto get_models_cmd =
        models_cmd->add_subcommand("get", "Get info of {model_id} locally");
    get_models_cmd->add_option("model_id", model_id, "");
    get_models_cmd->callback([&model_id]() {
      commands::ModelGetCmd command(model_id);
      command.Exec();
    });

    auto model_pull_cmd =
        app_.add_subcommand("pull",
                            "Download a model from a registry. Working with "
                            "HuggingFace repositories. For available models, "
                            "please visit https://huggingface.co/cortexso");
    model_pull_cmd->add_option("model_id", model_id, "");

    model_pull_cmd->callback([&model_id]() {
      commands::CmdInfo ci(model_id);
      commands::ModelPullCmd command(ci.model_name, ci.branch);
      command.Exec();
    });

    auto remove_cmd =
        models_cmd->add_subcommand("remove", "Remove a model by ID locally");
    auto update_cmd =
        models_cmd->add_subcommand("update", "Update configuration of a model");
  }

  std::string msg;
  {
    auto chat_cmd =
        app_.add_subcommand("chat", "Send a chat request to a model");

    chat_cmd->add_option("model_id", model_id, "");
    chat_cmd->add_option("-m,--message", msg, "Message to chat with model");

    chat_cmd->callback([&model_id, &msg] {
      commands::CmdInfo ci(model_id);
      std::string model_file =
          ci.branch == "main" ? ci.model_name : ci.model_name + "-" + ci.branch;
      config::YamlHandler yaml_handler;
      yaml_handler.ModelConfigFromFile(cortex_utils::GetCurrentPath() +
                                       "/models/" + model_file + ".yaml");
      commands::ChatCmd cc("127.0.0.1", 3928, yaml_handler.GetModelConfig());
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

    auto get_engine_cmd = engines_cmd->add_subcommand("get", "Get an engine");

    EngineInstall(engines_cmd, "cortex.llamacpp", version);
    EngineInstall(engines_cmd, "cortex.onnx", version);
    EngineInstall(engines_cmd, "cortex.tensorrt-llm", version);
  }

  {
    // cortex run tinyllama:gguf
    auto run_cmd =
        app_.add_subcommand("run", "Shortcut to start a model and chat");
    std::string model_id;
    run_cmd->add_option("model_id", model_id, "");
    run_cmd->callback([&model_id] {
      commands::RunCmd rc("127.0.0.1", 3928, model_id);
      rc.Exec();
    });
  }

  auto stop_cmd = app_.add_subcommand("stop", "Stop the API server");

  stop_cmd->callback([] {
    // TODO get info from config file
    commands::ServerStopCmd ssc("127.0.0.1", 3928);
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
  app_.add_flag_function("-v", cb, "Cortex version");

  std::string cortex_version;
  bool check_update = true;
  {
    auto update_cmd = app_.add_subcommand("update", "Update cortex version");

    update_cmd->add_option("-v", cortex_version, "");
    update_cmd->callback([&cortex_version, &check_update] {
      commands::CortexUpdCmd cuc;
      cuc.Exec();
      check_update = false;
    });
  }

  CLI11_PARSE(app_, argc, argv);

  // Check new update, only check for stable release for now
#ifdef CORTEX_CPP_VERSION
  if (check_update) {
    constexpr auto github_host = "https://api.github.com";
    std::ostringstream release_path;
    release_path << "/repos/janhq/cortex.cpp/releases/latest";
    CTL_INF("Engine release path: " << github_host << release_path.str());

    httplib::Client cli(github_host);
    if (auto res = cli.Get(release_path.str())) {
      if (res->status == httplib::StatusCode::OK_200) {
        try {
          auto json_res = nlohmann::json::parse(res->body);
          std::string latest_version = json_res["tag_name"].get<std::string>();
          std::string current_version = CORTEX_CPP_VERSION;
          if (current_version != latest_version) {
            CLI_LOG("\nA new release of cortex is available: "
                    << current_version << " -> " << latest_version);
            CLI_LOG("To upgrade, run: cortex update");
            CLI_LOG(json_res["html_url"].get<std::string>());
          }
        } catch (const nlohmann::json::parse_error& e) {
          CTL_INF("JSON parse error: " << e.what());
        }
      } else {
        CTL_INF("HTTP error: " << res->status);
      }
    } else {
      auto err = res.error();
      CTL_INF("HTTP error: " << httplib::to_string(err));
    }
  }
#endif

  return true;
}

void CommandLineParser::EngineInstall(CLI::App* parent,
                                      const std::string& engine_name,
                                      std::string& version) {
  auto engine_cmd =
      parent->add_subcommand(engine_name, "Manage " + engine_name + " engine");

  auto install_cmd = engine_cmd->add_subcommand(
      "install", "Install " + engine_name + " engine");
  install_cmd->add_option("-v, --version", version,
                          "Engine version. Default will be latest");

  install_cmd->callback([engine_name, &version] {
    commands::EngineInitCmd eic(engine_name, version);
    eic.Exec();
  });
}