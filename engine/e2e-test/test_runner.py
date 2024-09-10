import platform
import subprocess
import os
from typing import List, Tuple


def run(name: str, arguments: List[str]):
    if platform.system() == "Windows":
        executable = "build\\cortex-cpp.exe"
    else:
        executable = "build/cortex-cpp"
    result = subprocess.run([executable] + arguments, capture_output=True, text=True)
    return result.returncode, result.stdout, result.stderr
