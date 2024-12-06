import pytest
import sys

### e2e tests are expensive, have to keep engines tests in order
from test_api_engine_list import TestApiEngineList
from test_cli_engine_install_nightly import TestApiEngine
from test_api_model import TestApiModel
from test_api_model_import import TestApiModelImport

###
from test_cli_engine_get import TestCliEngineGet
from test_cli_engine_install_nightly import TestCliEngineInstall
from test_cli_engine_list import TestCliEngineList
from test_cli_engine_uninstall import TestCliEngineUninstall
from test_cli_model import TestCliModel
from test_cli_server_start import TestCliServerStart
from test_cortex_update import TestCortexUpdate
from test_create_log_folder import TestCreateLogFolder
from test_cli_model_import import TestCliModelImport

if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))
