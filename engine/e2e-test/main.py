import pytest
import sys
### e2e tests are expensive, have to keep engines tests in order
from test_api_engine_list import TestApiEngineList
from test_api_engine_install import TestApiEngineInstall
from test_api_engine_get import TestApiEngineGet
from test_api_engine_uninstall import TestApiEngineUninstall
### models, keeps in order
from test_api_model_pull_direct_url import TestApiModelPullDirectUrl
from test_api_model_start import TestApiModelStart
from test_api_model_stop import TestApiModelStop
from test_api_model_get import TestApiModelGet
from test_api_model_alias import TestApiModelAlias
from test_api_model_list import TestApiModelList
from test_api_model_update import TestApiModelUpdate
from test_api_model_delete import TestApiModelDelete
from test_api_model_import import TestApiModelImport
###
from test_cli_engine_get import TestCliEngineGet
from test_cli_engine_install import TestCliEngineInstall
from test_cli_engine_list import TestCliEngineList
from test_cli_engine_uninstall import TestCliEngineUninstall
from test_cli_model_delete import TestCliModelDelete
from test_cli_model_pull_direct_url import TestCliModelPullDirectUrl
from test_cli_server_start import TestCliServerStart
from test_cortex_update import TestCortexUpdate
from test_create_log_folder import TestCreateLogFolder
from test_cli_model_import import TestCliModelImport

if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))
