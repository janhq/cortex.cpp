import pytest

from test_cli_engine_list import TestCliEngineList
from test_cli_engine_get import TestCliEngineGet

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
