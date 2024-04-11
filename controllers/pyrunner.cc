#include "pyrunner.h"
#include <cstdio>
#include "utils/nitro_utils.h"
void workers::pyrunner::testrun(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {


  Json::Value jsonResp;
  jsonResp["message"] = "Python test run succesfully done";
  auto response = nitro_utils::nitroHttpJsonResponse(jsonResp);
  callback(response);
  return;
};

void workers::pyrunner::ExecutePythonCode(const std::string& PyModulePath, const std::string& PyEntryPoint) {
    std::string importCommand = "import sys\n"
                                "sys.path = []\n"
                                "sys.path.append('" + PyModulePath + "/deps/site-packages')\n"
                                "sys.path.append('./lib')\n"
                                "sys.path.append('./python/'+f'python{sys.version_info.major}.{sys.version_info.minor}')\n"
                                "sys.path.append('./python/'+f'python{sys.version_info.major}.{sys.version_info.minor}'+'/lib-dynload')\n"
                                "print(f'Python Version: {sys.version}')\n" // Print Python version
                                "print(f'Library Directories: {sys.path}')\n"; // Print library directories

        if (!Py_IsInitialized()) {
        Py_Initialize();
    }

PyGILState_STATE gilState = PyGILState_Ensure();
PyThreadState* mySubState = Py_NewInterpreter();
    // Execute the constructed command
    if (PyRun_SimpleString(importCommand.c_str()) != 0) {
        fprintf(stderr, "Python script execution failed.\n");
    }

    std::string fileEntry = PyModulePath + "/" + PyEntryPoint;
    FILE* file = fopen(fileEntry.c_str(), "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open Python entry point file.\n");
    } else {
        if (PyRun_SimpleFile(file, fileEntry.c_str()) != 0) {
            PyErr_Print();
            fprintf(stderr, "Python script file execution failed.\n");
        }
        fclose(file);
    }

Py_EndInterpreter(mySubState);
PyGILState_Release(gilState);

    PyGILState_Release(gilState);

}

void workers::pyrunner::PyRunPath(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) {

    const auto& jsonBody = req->getJsonObject();
    if (!jsonBody) {
        Json::Value jsonResp;
        jsonResp["message"] = "Json body is empty";
        callback(nitro_utils::nitroHttpJsonResponse(jsonResp));
        return;
    }

    std::string PyModulePath = jsonBody->operator[]("py_module_path").asString();
    std::string PyEntryPoint = jsonBody->get("entrypoint", "main.py").asString();

    if (PyModulePath.empty()) {
        Json::Value jsonResp;
        jsonResp["message"] = "No specified PyModulePath";
        callback(nitro_utils::nitroHttpJsonResponse(jsonResp));
        return;
    }

    // Run the Python code in a separate thread
    std::thread pythonThread(&workers::pyrunner::ExecutePythonCode, this, PyModulePath, PyEntryPoint);
    if (pythonThread.joinable()) {
        pythonThread.join(); // Wait for the thread to finish
    }

    Json::Value jsonResp;
    jsonResp["message"] = "Python test run successfully done";
    callback(nitro_utils::nitroHttpJsonResponse(jsonResp));
}
