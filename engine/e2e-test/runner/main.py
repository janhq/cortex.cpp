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
from api.engines.test_api_get_list_engine import TestApiEngineList
from api.engines.test_api_engine import TestApiEngine
from api.engines.test_api_get_default_engine import TestApiDefaultEngine
from api.engines.test_api_get_engine_release import TestApiEngineRelease
from api.engines.test_api_get_engine_release_latest import TestApiEngineReleaseLatest
from test_api_post_default_engine import TestApiSetDefaultEngine
from api.model.test_api_model import TestApiModel
from api.model.test_api_model_import import TestApiModelImport
from api.files.test_api_create_file import TestApiCreateFile
from api.files.test_api_get_file import TestApiGetFile
from api.files.test_api_get_list_file import TestApiGetListFile
from api.files.test_api_delete_file import TestApiDeleteFile
from api.message.test_api_get_message import TestApiGetMessage
from api.message.test_api_get_list_message import TestApiGetListMessage
from api.message.test_api_create_message import TestApiCreateMessage
from api.message.test_api_delete_message import TestApiDeleteMessage
from api.thread.test_api_create_thread import TestApiCreateThread
from api.thread.test_api_delete_thread import TestApiDeleteThread
from api.thread.test_api_get_thread import TestApiGetThread
from api.thread.test_api_get_list_thread import TestApiGetListThread

###
from cli.engines.test_cli_engine_get import TestCliEngineGet
from cli.engines.test_cli_engine_install import TestCliEngineInstall
from cli.engines.test_cli_engine_list import TestCliEngineList
from cli.engines.test_cli_engine_uninstall import TestCliEngineUninstall
from cli.model.test_cli_model import TestCliModel
from cli.model.test_cli_model_import import TestCliModelImport
from cli.common.test_cli_server_start import TestCliServerStart
from cli.common.test_cortex_update import TestCortexUpdate
from cli.common.test_create_log_folder import TestCreateLogFolder

if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-v"]))
