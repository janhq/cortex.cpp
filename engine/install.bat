@echo off
setlocal

:: Remove existing cortex-cpp directory if it exists
if exist "%APPDATA%\cortex-cpp" (
    echo Removing existing cortex-cpp installation...
    rmdir /S /Q "%APPDATA%\cortex-cpp"
)

:: Parse arguments
set "VERSION=latest"
set "GPU=false"
set "AVX=-avx2"

echo Please specify the desired AVX version (avx, avx2, avx512), or leave it blank to use the default version (avx2):
set /p USER_AVX=

if not "%USER_AVX%"=="" (
    set "AVX=-%USER_AVX%"
)

:arg_loop
if "%~1"=="" goto arg_loop_end
if "%~1"=="--gpu" (
    set "GPU=true"
    shift
    goto arg_loop
)
if "%~1"=="--version" (
    set "VERSION=%~2"
    shift
    shift
    goto arg_loop
)
shift
goto arg_loop
:arg_loop_end

echo %VERSION%

:: Get the release
if "%VERSION%"=="latest" (
    :: If the version is set to "latest", get the latest version number from the cortex-cpp GitHub repository
    for /f "delims=" %%i in ('powershell -Command "& {$version = Invoke-RestMethod -Uri 'https://api.github.com/repos/menloresearch/cortex/releases/latest'; return $version.tag_name.TrimStart('v')}"') do set "VERSION=%%i"
)

:: Construct the download URL
set "URL=https://github.com/menloresearch/cortex.cpp/releases/download/v%VERSION%/cortex-cpp-%VERSION%-win-amd64%AVX%"
if "%GPU%"=="true" (
    :: If --gpu option is provided, append -cuda to the URL
    set "URL=%URL%-cuda"
)
set "URL=%URL%.tar.gz"

:: Download and extract cortex-cpp
echo Downloading cortex-cpp from: %URL%
powershell -Command "Invoke-WebRequest -OutFile '%TEMP%\cortex-cpp.tar.gz' '%URL%'"
echo Extracting cortex-cpp...
powershell -Command "mkdir '%APPDATA%\cortex-cpp'"
powershell -Command "tar -zxvf '%TEMP%\cortex-cpp.tar.gz' -C '%APPDATA%\cortex-cpp'"

:: Add cortex-cpp to the PATH
setx PATH "%APPDATA%\cortex-cpp;%PATH%"

:: Create uninstallcortex-cpp.bat
echo @echo off > "%APPDATA%\cortex-cpp\uninstallcortex-cpp.bat"
echo setx PATH "%PATH:;%APPDATA%\cortex-cpp=;%"" >> "%APPDATA%\cortex-cpp\uninstallcortex-cpp.bat"
echo rmdir /S /Q "%APPDATA%\cortex-cpp" >> "%APPDATA%\cortex-cpp\uninstallcortex-cpp.bat"

:: Clean up
del %TEMP%\cortex-cpp.tar.gz

endlocal
