#include "model_alias_cmd.h"
#include "database/models.h"

namespace commands {

void ModelAliasCmd::Exec(const std::string& model_handle,
                         const std::string& model_alias) {
  cortex::db::Models modellist_handler;
  try {
    auto result = modellist_handler.UpdateModelAlias(model_handle, model_alias);
    if (result.has_error()) {
      CLI_LOG(result.error());
    } else {
      if (result.value()) {
        CLI_LOG("Successfully set model alias '" + model_alias +
                "' for modeID '" + model_handle + "'.");
      } else {
        CLI_LOG("Unable to set model alias for modelID '" + model_handle +
                "': model alias '" + model_alias + "' is not unique!");
      }
    }

  } catch (const std::exception& e) {
    CLI_LOG("Error when setting model alias ('" + model_alias +
            "') for modelID '" + model_handle + "':" + e.what());
  }
}

}  // namespace commands