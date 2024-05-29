@echo off

setlocal enabledelayedexpansion

set "TEMP=C:\Users\%UserName%\AppData\Local\Temp"

rem Check for required arguments
if "%~2"=="" (
    echo Usage: %~0 ^<path_to_binary^> ^<path_to_python_file^>
    exit /b 1
)

set "BINARY_PATH=%~1"
set "PYTHON_FILE_EXECUTION_PATH=%~2"

for %%i in ("%BINARY_PATH%") do set "BINARY_NAME=%%~nxi"

echo BINARY_NAME=%BINARY_NAME%

del %TEMP%\response1.log 2>nul
del %TEMP%\server.log 2>nul

set /a min=9999
set /a max=11000
set /a range=max-min+1
set /a PORT=%min% + %RANDOM% %% %range%

rem Install numpy for Python
set "PYTHONHOME=%cd%\engines\cortex.python\python"
echo Set Python HOME to %PYTHONHOME%
%PYTHONHOME%\python.exe -m ensurepip
%PYTHONHOME%\python.exe -m pip install --upgrade pip
%PYTHONHOME%\python.exe -m pip install numpy --target=%PYTHONHOME%\Lib\site-packages\

rem Start the binary file
start "" /B "%BINARY_PATH%" 1 "127.0.0.1" %PORT%  > "%TEMP%\server.log" 2>&1

ping -n 3 127.0.0.1 > nul

rem Capture the PID of the started process with "server" in its name
for /f "tokens=2" %%a in ('tasklist /fi "imagename eq %BINARY_NAME%" /fo list ^| findstr /B "PID:"') do (
    set "pid=%%a"
)

echo pid=%pid%

if not defined pid (
    echo server failed to start. Logs:
    type %TEMP%\server.log
    echo.
    exit /b 1
)

rem Wait for a few seconds to let the server start

rem Define JSON strings for curl data
call set "PYTHON_FILE_EXECUTION_PATH_STRING=%%PYTHON_FILE_EXECUTION_PATH:\=\\%%"
set "curl_data1={\"file_execution_path\":\"%PYTHON_FILE_EXECUTION_PATH_STRING%\"}"

rem Print the values of curl_data for debugging
echo curl_data1=%curl_data1%

rem Run the curl commands and capture the status code
curl.exe --connect-timeout 60 -o "%TEMP%\response1.log" -s -w "%%{http_code}" --location "http://127.0.0.1:%PORT%/v1/fine_tuning/job" --header "Content-Type: application/json" --data "%curl_data1%" > %TEMP%\response1.log 2>&1

set "error_occurred=0"

rem Read the status code directly from the response file
set "response1="
for /f %%a in (%TEMP%\response1.log) do set "response1=%%a"

if "%response1%" neq "200" (
    echo The first curl command failed with status code: %response1%
    type %TEMP%\response1.log
    echo.
    set "error_occurred=1"
)

echo ----------------------
echo Log python file execution:
type %TEMP%\response1.log
echo.

rem Verification step: Check the contents of output.txt
set "expected_output=1 2 3"
set "actual_output="
if exist "output.txt" (
    for /f "delims=" %%x in (output.txt) do set "actual_output=%%x"
    if "!actual_output!"=="!expected_output!" (
        echo Verification succeeded: output.txt contains the expected data.
    ) else (
        echo Verification failed: output.txt does not contain the expected data.
        echo Expected: !expected_output!
        echo Actual: !actual_output!
        set "error_occurred=1"
    )
) else (
    echo Verification failed: output.txt does not exist.
    set "error_occurred=1"
)

echo ----------------------
echo Server logs:
type %TEMP%\server.log
echo.

if "%error_occurred%"=="1" (
    echo Server test run failed!!!!!!!!!!!!!!!!!!!!!!
    taskkill /f /pid %pid%
    echo An error occurred while running the server.
    exit /b 1
)

echo Server test run successfully!

rem Kill the server process
taskkill /f /im server.exe 2>nul || exit /B 0

endlocal