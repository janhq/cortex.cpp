#include "model_pull_cmd.h"
#include <memory>
#include "cli/utils/easywsclient.hpp"
#include "cli/utils/indicators.hpp"
#include "common/event.h"
#include "database/models.h"
#include "server_start_cmd.h"
#include "utils/cli_selection_utils.h"
#include "utils/download_manager.h"
#include "utils/format_utils.h"
#include "utils/huggingface_utils.h"
#include "utils/json_helper.h"
#include "utils/logging_utils.h"
#include "utils/string_utils.h"

namespace commands {
namespace {
// TODO(sang) request from Server
cpp::result<std::string, std::string> GetModelId(const std::string& input) {
  if (input.empty()) {
    return cpp::fail(
        "Input must be Cortex Model Hub handle or HuggingFace url!");
  }

  if (string_utils::StartsWith(input, "https://")) {
    return input;
  }

  if (input.find(":") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, ":");
    if (parsed.size() != 2) {
      return cpp::fail("Invalid model handle: " + input);
    }
    return input;
  }

  if (input.find("/") != std::string::npos) {
    auto parsed = string_utils::SplitBy(input, "/");
    if (parsed.size() != 2) {
      return cpp::fail("Invalid model handle: " + input);
    }

    auto author = parsed[0];
    auto model_name = parsed[1];
    if (author == "cortexso") {
      return author + ":" + model_name;
    }

    auto repo_info =
        huggingface_utils::GetHuggingFaceModelRepoInfo(author, model_name);

    if (!repo_info.has_value()) {
      return cpp::fail("Model not found");
    }

    if (!repo_info->gguf.has_value()) {
      return cpp::fail(
          "Not a GGUF model. Currently, only GGUF single file is "
          "supported.");
    }

    std::vector<std::string> options{};
    for (const auto& sibling : repo_info->siblings) {
      if (string_utils::EndsWith(sibling.rfilename, ".gguf")) {
        options.push_back(sibling.rfilename);
      }
    }
    auto selection = cli_selection_utils::PrintSelection(options);
    std::cout << "Selected: " << selection.value() << std::endl;

    return huggingface_utils::GetDownloadableUrl(author, model_name,
                                                 selection.value());
  }
  auto branches =
      huggingface_utils::GetModelRepositoryBranches("cortexso", input);
  if (branches.has_error()) {
    return cpp::fail(branches.error());
  }

  auto default_model_branch = huggingface_utils::GetDefaultBranch(input);

  cortex::db::Models modellist_handler;
  auto downloaded_model_ids =
      modellist_handler.FindRelatedModel(input).value_or(
          std::vector<std::string>{});

  std::vector<std::string> avai_download_opts{};
  for (const auto& branch : branches.value()) {
    if (branch.second.name == "main") {  // main branch only have metadata. skip
      continue;
    }
    auto model_id = input + ":" + branch.second.name;
    if (std::find(downloaded_model_ids.begin(), downloaded_model_ids.end(),
                  model_id) !=
        downloaded_model_ids.end()) {  // if downloaded, we skip it
      continue;
    }
    avai_download_opts.emplace_back(model_id);
  }

  if (avai_download_opts.empty()) {
    // TODO: only with pull, we return
    return cpp::fail("No variant available");
  }
  std::optional<std::string> normalized_def_branch = std::nullopt;
  if (default_model_branch.has_value()) {
    normalized_def_branch = input + ":" + default_model_branch.value();
  }
  string_utils::SortStrings(downloaded_model_ids);
  string_utils::SortStrings(avai_download_opts);
  auto selection = cli_selection_utils::PrintModelSelection(
      downloaded_model_ids, avai_download_opts, normalized_def_branch);
  if (!selection.has_value()) {
    return cpp::fail("Invalid selection");
  }
  return selection.value();
}

}  // namespace
void ModelPullCmd::Exec(const std::string& host, int port,
                        const std::string& input) {
  auto r = GetModelId(input);
  if (r.has_error()) {
    CLI_LOG(r.error());
    return;
  }
  auto const& model_id = r.value();
  CTL_INF(model_id);
  // Start server if server is not started yet
  if (!commands::IsServerAlive(host, port)) {
    CLI_LOG("Starting server ...");
    commands::ServerStartCmd ssc;
    if (!ssc.Exec(host, port)) {
      return;
    }
  }

  httplib::Client cli(host + ":" + std::to_string(port));
  Json::Value json_data;
  json_data["model"] = model_id;
  auto data_str = json_data.toStyledString();
  cli.set_read_timeout(std::chrono::seconds(60));
  auto res = cli.Post("/v1/models/pull", httplib::Headers(), data_str.data(),
                      data_str.size(), "application/json");

  if (res) {
    if (res->status == httplib::StatusCode::OK_200) {
      // CLI_LOG("OK");
    } else {
      auto root = json_helper::ParseJsonString(res->body);
      CLI_LOG(root["message"].asString());
      return;
    }
  } else {
    auto err = res.error();
    CTL_ERR("HTTP error: " << httplib::to_string(err));
    return;
  }

  CLI_LOG("Pulling ...")
  DownloadManager dm;
  dm.Connect(host, port);
  if (!dm.Handle(model_id))
    return;

  CLI_LOG("Model " << model_id << " downloaded successfully!")
}
};  // namespace commands
