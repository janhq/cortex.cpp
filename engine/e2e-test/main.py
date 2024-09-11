import pytest
from test_api_engine_list import TestApiEngineList
from test_cli_engine_get import TestCliEngineGet
from test_cli_engine_list import TestCliEngineList

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
