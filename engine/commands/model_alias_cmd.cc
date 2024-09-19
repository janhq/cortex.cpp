#include "model_alias_cmd.h"
#include "utils/modellist_utils.h"

namespace commands {

void ModelAliasCmd::Exec(const std::string& model_handle,
                         const std::string& model_alias) {
  modellist_utils::ModelListUtils modellist_handler;
  try {
    if (modellist_handler.UpdateModelAlias(model_handle, model_alias)) {
      CLI_LOG("Successfully set model alias '" + model_alias +
              "' for modeID '" + model_handle + "'.");
    } else {
      CLI_LOG("Unable to set model alias for modelID '" + model_handle +
              "': model alias '" + model_alias + "' or modelID is not unique!");
    }
  } catch (const std::exception& e) {
    CLI_LOG("Error when setting model alias ('" + model_alias +
            "') for modelID '" + model_handle + "':" + e.what());
  }
}

}  // namespace commands