import platform
import os
from pathlib import Path
import pytest, requests, shutil
from utils.test_runner import run
from utils.test_runner import start_server, stop_server


def get_root_path():
    if platform.system() == "Linux":
        # For Linux, use the XDG base directory.
        # Here we use XDG_DATA_HOME if set, otherwise default to ~/.local/share.
        return Path(os.environ.get("XDG_DATA_HOME", Path.home() / ".local" / "share"))
    else:
        return Path.home()


class TestCreateLogFolder:
    @pytest.fixture(autouse=True)
    def setup_and_teardown(self):
        # Setup
        stop_server()
        root = get_root_path()
        if os.path.exists(root / "cortexcpp" / "logs"):
            shutil.rmtree(root / "cortexcpp" / "logs")
        success = start_server()
        if not success:
            raise Exception("Failed to start server")

        yield

        # Teardown
        stop_server()

    def test_create_log_folder_run_successfully(self):
        root = get_root_path()
        assert (
            os.path.exists(root / "cortexcpp" / "logs")
            or os.path.exists(root / "cortexcpp-beta" / "logs")
            or os.path.exists(root / "cortexcpp-nightly" / "logs")
        )
