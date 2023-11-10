@echo off

set "TEMP=C:\Users\%UserName%\AppData\Local\Temp"
set "MODEL_PATH=%TEMP%\testmodel"

rem Check for required arguments
if "%~2"=="" (
    echo Usage: %~0 ^<path_to_binary^> ^<url_to_download^>
    exit /b 1
)

set "BINARY_PATH=%~1"
set "DOWNLOAD_URL=%~2"

for %%i in ("%BINARY_PATH%") do set "BINARY_NAME=%%~nxi"

echo BINARY_NAME=%BINARY_NAME%

del %TEMP%\response1.log 2>nul
del %TEMP%\response2.log 2>nul
del %TEMP%\nitro.log 2>nul

rem Start the binary file
start /B "" "%BINARY_PATH%" > %TEMP%\nitro.log 2>&1

ping -n 6 127.0.0.1 > nul

rem Capture the PID of the started process with "nitro" in its name
for /f "tokens=2" %%a in ('tasklist /fi "imagename eq %BINARY_NAME%" /fo list ^| findstr /B "PID:"') do (
    set "pid=%%a"
)

echo pid=%pid%

if not defined pid (
    echo nitro failed to start. Logs:
    type %TEMP%\nitro.log
    exit /b 1
)

rem Wait for a few seconds to let the server start

rem Check if %TEMP%\testmodel exists, if not, download it
if not exist "%MODEL_PATH%" (
    bitsadmin.exe /transfer "DownloadTestModel" %DOWNLOAD_URL% "%MODEL_PATH%"
)

rem Define JSON strings for curl data
call set "MODEL_PATH_STRING=%%MODEL_PATH:\=\\%%"
set "curl_data1={\"llama_model_path\":\"%MODEL_PATH_STRING%\"}"
set "curl_data2={\"messages\":[{\"content\":\"Hello there\",\"role\":\"assistant\"},{\"content\":\"Write a long and sad story for me\",\"role\":\"user\"}],\"stream\":true,\"model\":\"gpt-3.5-turbo\",\"max_tokens\":100,\"stop\":[\"hello\"],\"frequency_penalty\":0,\"presence_penalty\":0,\"temperature\":0.7}"

rem Print the values of curl_data1 and curl_data2 for debugging
echo curl_data1=%curl_data1%
echo curl_data2=%curl_data2%

rem Run the curl commands and capture the status code
curl.exe -o %TEMP%\response1.log -s -w "%%{http_code}" --location "http://localhost:3928/inferences/llamacpp/loadModel" --header "Content-Type: application/json" --data "%curl_data1%" > %TEMP%\response1_code.log 2>&1

curl.exe -o %TEMP%\response2.log -s -w "%%{http_code}" --location "http://localhost:3928/inferences/llamacpp/chat_completion" ^
--header "Content-Type: application/json" ^
--header "Accept: text/event-stream" ^
--header "Access-Control-Allow-Origin: *" ^
--data "%curl_data2%" > %TEMP%\response2_code.log 2>&1

set "error_occurred=0"

rem Read the status codes from the log files
for /f %%a in (%TEMP%\response1_code.log) do set "response1=%%a"
for /f %%a in (%TEMP%\response2_code.log) do set "response2=%%a"

if "%response1%" neq "200" (
    echo The first curl command failed with status code: %response1%
    type %TEMP%\response1.log
    set "error_occurred=1"
)

if "%response2%" neq "200" (
    echo The second curl command failed with status code: %response2%
    type %TEMP%\response2.log
    set "error_occurred=1"
)

if "%error_occurred%"=="1" (
    echo Nitro test run failed!!!!!!!!!!!!!!!!!!!!!!
    echo Nitro Error Logs:
    type %TEMP%\nitro.log
    taskkill /f /pid %pid%
    exit /b 1
)


echo ----------------------
echo Log load model:
type %TEMP%\response1.log

echo ----------------------
echo "Log run test:"
type %TEMP%\response2.log

echo Nitro test run successfully!

rem Kill the server process
taskkill /f /pid %pid%
