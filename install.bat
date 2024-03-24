@echo off
setlocal

:: Remove existing nitro directory if it exists
if exist "%APPDATA%\nitro" (
    echo Removing existing Nitro installation...
    rmdir /S /Q "%APPDATA%\nitro"
)

:: Parse arguments
set "VERSION=latest"
set "GPU=false"
set "AVX=-avx2"

echo Please enter the AVX version you want (avx, avx2, avx512) or leave blank for default (avx2):
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
    :: If the version is set to "latest", get the latest version number from the Nitro GitHub repository
    for /f "delims=" %%i in ('powershell -Command "& {$version = Invoke-RestMethod -Uri 'https://api.github.com/repos/janhq/nitro/releases/latest'; return $version.tag_name.TrimStart('v')}"') do set "VERSION=%%i"
)

:: Construct the download URL
set "URL=https://github.com/janhq/nitro/releases/download/v%VERSION%/nitro-%VERSION%-win-amd64%AVX%"
if "%GPU%"=="true" (
    :: If --gpu option is provided, append -cuda to the URL
    set "URL=%URL%-cuda"
)
set "URL=%URL%.tar.gz"

:: Download and extract nitro
echo Downloading Nitro from: %URL%
powershell -Command "Invoke-WebRequest -OutFile '%TEMP%\nitro.tar.gz' '%URL%'"
echo Extracting Nitro...
powershell -Command "mkdir '%APPDATA%\nitro'"
powershell -Command "tar -zxvf '%TEMP%\nitro.tar.gz' -C '%APPDATA%\nitro'"

:: Add nitro to the PATH
setx PATH "%APPDATA%\nitro;%PATH%"

:: Create uninstallnitro.bat
echo @echo off > "%APPDATA%\nitro\uninstallnitro.bat"
echo setx PATH "%PATH:;%APPDATA%\nitro=;%"" >> "%APPDATA%\nitro\uninstallnitro.bat"
echo rmdir /S /Q "%APPDATA%\nitro" >> "%APPDATA%\nitro\uninstallnitro.bat"

:: Clean up
del %TEMP%\nitro.tar.gz

endlocal
