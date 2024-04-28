@echo off
pushd %~dp0
cd ..
SET INSTALL_DIR=%CD%\build_deps\_install\python

echo Creating install directory...
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
cd %INSTALL_DIR%

echo Downloading Python...
curl -o python-installer.exe https://www.python.org/ftp/python/3.10.4/python-3.10.4-amd64.exe

echo Installing Python to %INSTALL_DIR%
start /wait python-installer.exe /quiet InstallAllUsers=0 PrependPath=0 TargetDir=%INSTALL_DIR% Include_pip=1 Include_test=0

if not exist "%INSTALL_DIR%\Lib" (
    echo Installation failed, attempting to repair...
    start /wait python-installer.exe /quiet InstallAllUsers=0 PrependPath=0 TargetDir=%INSTALL_DIR% Include_pip=1 Include_test=0 /repair 
)

if not exist "%INSTALL_DIR%\python.exe" (
    echo Error: Failed to install Python3
    echo Please clear the build cache or repair Python3 using python-installer.exe in 'cortex\build_deps\_install\python\'
) else (
    echo Verifying Python Installation...
    "%INSTALL_DIR%\python.exe" --version
)

echo Done.
popd
