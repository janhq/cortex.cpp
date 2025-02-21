import pytest
import sys
import os

# Add the project root to sys.path
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


# Add all necessary paths
sys.path.append(PROJECT_ROOT)               
sys.path.append(os.path.join(PROJECT_ROOT, "api/engines"))  
sys.path.append(os.path.join(PROJECT_ROOT, "api/hub"))  
sys.path.append(os.path.join(PROJECT_ROOT, "api/model"))  
sys.path.append(os.path.join(PROJECT_ROOT, "cli/engines"))  
sys.path.append(os.path.join(PROJECT_ROOT, "cli/model"))  
sys.path.append(os.path.join(PROJECT_ROOT, "cli/common"))  

### e2e tests are expensive, have to keep engines tests in order
from test_api_get_list_engine import TestApiEngineList
from test_api_engine_install_nightly import TestApiEngineInstall
from test_api_model import TestApiModel
from test_api_model_import import TestApiModelImport

###
from test_cli_engine_get import TestCliEngineGet
from test_cli_engine_install_nightly import TestCliEngineInstall
from test_cli_engine_list import TestCliEngineList
from test_cli_engine_uninstall import TestCliEngineUninstall
from test_cli_model import TestCliModel
from test_cli_model_import import TestCliModelImport
from test_cli_server_start import TestCliServerStart
from test_cortex_update import TestCortexUpdate
from test_create_log_folder import TestCreateLogFolder

if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))
