"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var tslib_1 = require("tslib");
var node_os_1 = tslib_1.__importDefault(require("node:os"));
var node_fs_1 = tslib_1.__importDefault(require("node:fs"));
var node_path_1 = tslib_1.__importDefault(require("node:path"));
var node_child_process_1 = require("node:child_process");
var tcp_port_used_1 = tslib_1.__importDefault(require("tcp-port-used"));
var fetch_retry_1 = tslib_1.__importDefault(require("fetch-retry"));
var os_utils_1 = tslib_1.__importDefault(require("os-utils"));
var nvidia_1 = require("./nvidia");
var execute_1 = require("./execute");
// Polyfill fetch with retry
var fetchRetry = (0, fetch_retry_1.default)(fetch);
// The PORT to use for the Nitro subprocess
var PORT = 3928;
// The HOST address to use for the Nitro subprocess
var LOCAL_HOST = "127.0.0.1";
// The URL for the Nitro subprocess
var NITRO_HTTP_SERVER_URL = "http://".concat(LOCAL_HOST, ":").concat(PORT);
// The URL for the Nitro subprocess to load a model
var NITRO_HTTP_LOAD_MODEL_URL = "".concat(NITRO_HTTP_SERVER_URL, "/inferences/llamacpp/loadmodel");
// The URL for the Nitro subprocess to validate a model
var NITRO_HTTP_VALIDATE_MODEL_URL = "".concat(NITRO_HTTP_SERVER_URL, "/inferences/llamacpp/modelstatus");
// The URL for the Nitro subprocess to kill itself
var NITRO_HTTP_KILL_URL = "".concat(NITRO_HTTP_SERVER_URL, "/processmanager/destroy");
// The URL for the Nitro subprocess to run chat completion
var NITRO_HTTP_CHAT_URL = "".concat(NITRO_HTTP_SERVER_URL, "/inferences/llamacpp/chat_completion");
// The default config for using Nvidia GPU
var NVIDIA_DEFAULT_CONFIG = {
    notify: true,
    run_mode: "cpu",
    nvidia_driver: {
        exist: false,
        version: "",
    },
    cuda: {
        exist: false,
        version: "",
    },
    gpus: [],
    gpu_highest_vram: "",
};
// The supported model format
// TODO: Should be an array to support more models
var SUPPORTED_MODEL_FORMATS = [".gguf"];
// The subprocess instance for Nitro
var subprocess = undefined;
// The current model file url
var currentModelFile = "";
// The current model settings
var currentSettings = undefined;
// The Nvidia info file for checking for CUDA support on the system
var nvidiaConfig = NVIDIA_DEFAULT_CONFIG;
// The logger to use, default to stdout
var log = function (message) {
    var _ = [];
    for (var _i = 1; _i < arguments.length; _i++) {
        _[_i - 1] = arguments[_i];
    }
    return process.stdout.write(message + node_os_1.default.EOL);
};
/**
 * Get current Nvidia config
 * @returns {NitroNvidiaConfig} A copy of the config object
 * The returned object should be used for reading only
 * Writing to config should be via the function {@setNvidiaConfig}
 */
function getNvidiaConfig() {
    return Object.assign({}, nvidiaConfig);
}
/**
 * Set custom Nvidia config for running inference over GPU
 * @param {NitroNvidiaConfig} config The new config to apply
 */
function setNvidiaConfig(config) {
    nvidiaConfig = config;
}
/**
 * Set logger before running nitro
 * @param {NitroLogger} logger The logger to use
 */
function setLogger(logger) {
    log = logger;
}
/**
 * Stops a Nitro subprocess.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
function stopModel() {
    return killSubprocess();
}
/**
 * Initializes a Nitro subprocess to load a machine learning model.
 * @param modelFullPath - The absolute full path to model directory.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 * TODO: Should pass absolute of the model file instead of just the name - So we can modurize the module.ts to npm package
 */
function runModel(_a) {
    var modelFullPath = _a.modelFullPath, promptTemplate = _a.promptTemplate;
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        var files, ggufBinFile, nitroResourceProbe, prompt;
        return tslib_1.__generator(this, function (_b) {
            switch (_b.label) {
                case 0:
                    files = node_fs_1.default.readdirSync(modelFullPath);
                    ggufBinFile = files.find(function (file) {
                        return file === node_path_1.default.basename(modelFullPath) ||
                            SUPPORTED_MODEL_FORMATS.some(function (ext) { return file.toLowerCase().endsWith(ext); });
                    });
                    if (!ggufBinFile)
                        return [2 /*return*/, Promise.reject("No GGUF model file found")];
                    currentModelFile = node_path_1.default.join(modelFullPath, ggufBinFile);
                    return [4 /*yield*/, getResourcesInfo()];
                case 1:
                    nitroResourceProbe = _b.sent();
                    prompt = {};
                    if (promptTemplate) {
                        try {
                            Object.assign(prompt, promptTemplateConverter(promptTemplate));
                        }
                        catch (e) {
                            return [2 /*return*/, Promise.reject(e)];
                        }
                    }
                    currentSettings = tslib_1.__assign(tslib_1.__assign({}, prompt), { llama_model_path: currentModelFile, 
                        // This is critical and requires real system information
                        cpu_threads: Math.max(1, Math.round(nitroResourceProbe.numCpuPhysicalCore / 2)) });
                    return [2 /*return*/, runNitroAndLoadModel()];
            }
        });
    });
}
/**
 * 1. Spawn Nitro process
 * 2. Load model into Nitro subprocess
 * 3. Validate model status
 * @returns
 */
function runNitroAndLoadModel() {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        return tslib_1.__generator(this, function (_a) {
            // Gather system information for CPU physical cores and memory
            return [2 /*return*/, killSubprocess()
                    .then(function () { return tcp_port_used_1.default.waitUntilFree(PORT, 300, 5000); })
                    .then(function () {
                    /**
                     * There is a problem with Windows process manager
                     * Should wait for awhile to make sure the port is free and subprocess is killed
                     * The tested threshold is 500ms
                     **/
                    if (process.platform === "win32") {
                        return new Promise(function (resolve) { return setTimeout(function () { return resolve({}); }, 500); });
                    }
                    else {
                        return Promise.resolve({});
                    }
                })
                    .then(spawnNitroProcess)
                    .then(function () { return loadLLMModel(currentSettings); })
                    .then(validateModelStatus)
                    .catch(function (err) {
                    // TODO: Broadcast error so app could display proper error message
                    log("[NITRO]::Error: ".concat(err));
                    return { error: err };
                })];
        });
    });
}
/**
 * Parse prompt template into agrs settings
 * @param {string} promptTemplate Template as string
 * @returns {(NitroPromptSetting | never)} parsed prompt setting
 * @throws {Error} if cannot split promptTemplate
 */
function promptTemplateConverter(promptTemplate) {
    // Split the string using the markers
    var systemMarker = "{system_message}";
    var promptMarker = "{prompt}";
    if (promptTemplate.includes(systemMarker) &&
        promptTemplate.includes(promptMarker)) {
        // Find the indices of the markers
        var systemIndex = promptTemplate.indexOf(systemMarker);
        var promptIndex = promptTemplate.indexOf(promptMarker);
        // Extract the parts of the string
        var system_prompt = promptTemplate.substring(0, systemIndex);
        var user_prompt = promptTemplate.substring(systemIndex + systemMarker.length, promptIndex);
        var ai_prompt = promptTemplate.substring(promptIndex + promptMarker.length);
        // Return the split parts
        return { system_prompt: system_prompt, user_prompt: user_prompt, ai_prompt: ai_prompt };
    }
    else if (promptTemplate.includes(promptMarker)) {
        // Extract the parts of the string for the case where only promptMarker is present
        var promptIndex = promptTemplate.indexOf(promptMarker);
        var user_prompt = promptTemplate.substring(0, promptIndex);
        var ai_prompt = promptTemplate.substring(promptIndex + promptMarker.length);
        // Return the split parts
        return { user_prompt: user_prompt, ai_prompt: ai_prompt };
    }
    // Throw error if none of the conditions are met
    throw Error("Cannot split prompt template");
}
/**
 * Loads a LLM model into the Nitro subprocess by sending a HTTP POST request.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
function loadLLMModel(settings) {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        var res, err_1;
        return tslib_1.__generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    log("[NITRO]::Debug: Loading model with params ".concat(JSON.stringify(settings)));
                    _a.label = 1;
                case 1:
                    _a.trys.push([1, 4, , 6]);
                    return [4 /*yield*/, fetchRetry(NITRO_HTTP_LOAD_MODEL_URL, {
                            method: "POST",
                            headers: {
                                "Content-Type": "application/json",
                            },
                            body: JSON.stringify(settings),
                            retries: 3,
                            retryDelay: 500,
                        })];
                case 2:
                    res = _a.sent();
                    // FIXME: Actually check response, as the model directory might not exist
                    log("[NITRO]::Debug: Load model success with response ".concat(JSON.stringify(res)));
                    return [4 /*yield*/, Promise.resolve(res)];
                case 3: return [2 /*return*/, _a.sent()];
                case 4:
                    err_1 = _a.sent();
                    log("[NITRO]::Error: Load model failed with error ".concat(err_1));
                    return [4 /*yield*/, Promise.reject()];
                case 5: return [2 /*return*/, _a.sent()];
                case 6: return [2 /*return*/];
            }
        });
    });
}
/**
 * Run chat completion by sending a HTTP POST request and stream the response if outStream is specified
 * @param {any} request The request that is then sent to nitro
 * @param {WritableStream} outStream Optional stream that consume the response body
 * @returns {Promise<Response>} A Promise that resolves when the chat completion success, or rejects with an error if the completion fails.
 * @description If outStream is specified, the response body is consumed and cannot be used to reconstruct the data
 */
function chatCompletion(request, outStream) {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        var _this = this;
        return tslib_1.__generator(this, function (_a) {
            if (outStream) {
                // Add stream option if there is an outStream specified when calling this function
                Object.assign(request, {
                    stream: true,
                });
            }
            log("[NITRO]::Debug: Running chat completion with request ".concat(JSON.stringify(request)));
            return [2 /*return*/, fetchRetry(NITRO_HTTP_CHAT_URL, {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        Accept: "text/event-stream",
                        "Access-Control-Allow-Origin": "*",
                    },
                    body: JSON.stringify(request),
                    retries: 3,
                    retryDelay: 500,
                })
                    .then(function (response) { return tslib_1.__awaiter(_this, void 0, void 0, function () {
                    var outPipe;
                    return tslib_1.__generator(this, function (_a) {
                        switch (_a.label) {
                            case 0:
                                if (!outStream) return [3 /*break*/, 2];
                                if (!response.body) {
                                    throw new Error("Error running chat completion");
                                }
                                outPipe = response.body
                                    .pipeThrough(new TextDecoderStream())
                                    .pipeTo(outStream);
                                // Wait for all the streams to complete before returning from async function
                                return [4 /*yield*/, outPipe];
                            case 1:
                                // Wait for all the streams to complete before returning from async function
                                _a.sent();
                                _a.label = 2;
                            case 2:
                                log("[NITRO]::Debug: Chat completion success");
                                return [2 /*return*/, response];
                        }
                    });
                }); })
                    .catch(function (err) {
                    log("[NITRO]::Error: Chat completion failed with error ".concat(err));
                    throw err;
                })];
        });
    });
}
/**
 * Validates the status of a model.
 * @returns {Promise<NitroModelOperationResponse>} A promise that resolves to an object.
 * If the model is loaded successfully, the object is empty.
 * If the model is not loaded successfully, the object contains an error message.
 */
function validateModelStatus() {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        var _this = this;
        return tslib_1.__generator(this, function (_a) {
            // Send a GET request to the validation URL.
            // Retry the request up to 3 times if it fails, with a delay of 500 milliseconds between retries.
            return [2 /*return*/, fetchRetry(NITRO_HTTP_VALIDATE_MODEL_URL, {
                    method: "GET",
                    headers: {
                        "Content-Type": "application/json",
                    },
                    retries: 5,
                    retryDelay: 500,
                }).then(function (res) { return tslib_1.__awaiter(_this, void 0, void 0, function () {
                    var body;
                    return tslib_1.__generator(this, function (_a) {
                        switch (_a.label) {
                            case 0:
                                log("[NITRO]::Debug: Validate model state success with response ".concat(JSON.stringify(res)));
                                if (!res.ok) return [3 /*break*/, 2];
                                return [4 /*yield*/, res.json()];
                            case 1:
                                body = _a.sent();
                                // If the model is loaded, return an empty object.
                                // Otherwise, return an object with an error message.
                                if (body.model_loaded) {
                                    return [2 /*return*/, Promise.resolve({})];
                                }
                                _a.label = 2;
                            case 2: return [2 /*return*/, Promise.resolve({ error: "Validate model status failed" })];
                        }
                    });
                }); })];
        });
    });
}
/**
 * Terminates the Nitro subprocess.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
function killSubprocess() {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        var controller;
        return tslib_1.__generator(this, function (_a) {
            controller = new AbortController();
            setTimeout(function () { return controller.abort(); }, 5000);
            log("[NITRO]::Debug: Request to kill Nitro");
            return [2 /*return*/, fetch(NITRO_HTTP_KILL_URL, {
                    method: "DELETE",
                    signal: controller.signal,
                })
                    .then(function () {
                    subprocess === null || subprocess === void 0 ? void 0 : subprocess.kill();
                    subprocess = undefined;
                })
                    .catch(function (err) { return ({ error: err }); })
                    .then(function () { return tcp_port_used_1.default.waitUntilFree(PORT, 300, 5000); })
                    .then(function () { return log("[NITRO]::Debug: Nitro process is terminated"); })
                    .then(function () { return Promise.resolve({}); })];
        });
    });
}
/**
 * Spawns a Nitro subprocess.
 * @returns A promise that resolves when the Nitro subprocess is started.
 */
function spawnNitroProcess() {
    var _this = this;
    log("[NITRO]::Debug: Spawning Nitro subprocess...");
    return new Promise(function (resolve, reject) { return tslib_1.__awaiter(_this, void 0, void 0, function () {
        var binaryFolder, executableOptions, args;
        return tslib_1.__generator(this, function (_a) {
            binaryFolder = node_path_1.default.join(__dirname, "..", "bin");
            executableOptions = (0, execute_1.executableNitroFile)(nvidiaConfig);
            args = ["1", LOCAL_HOST, PORT.toString()];
            // Execute the binary
            log("[NITRO]::Debug: Spawn nitro at path: ".concat(executableOptions.executablePath, ", and args: ").concat(args));
            subprocess = (0, node_child_process_1.spawn)(executableOptions.executablePath, ["1", LOCAL_HOST, PORT.toString()], {
                cwd: binaryFolder,
                env: tslib_1.__assign(tslib_1.__assign({}, process.env), { CUDA_VISIBLE_DEVICES: executableOptions.cudaVisibleDevices }),
            });
            // Handle subprocess output
            subprocess.stdout.on("data", function (data) {
                log("[NITRO]::Debug: ".concat(data));
            });
            subprocess.stderr.on("data", function (data) {
                log("[NITRO]::Error: ".concat(data));
            });
            subprocess.on("close", function (code) {
                log("[NITRO]::Debug: Nitro exited with code: ".concat(code));
                subprocess = undefined;
                reject("child process exited with code ".concat(code));
            });
            tcp_port_used_1.default.waitUntilUsed(PORT, 300, 30000).then(function () {
                log("[NITRO]::Debug: Nitro is ready");
                resolve({});
            });
            return [2 /*return*/];
        });
    }); });
}
/**
 * Get the system resources information
 */
function getResourcesInfo() {
    var _this = this;
    return new Promise(function (resolve) { return tslib_1.__awaiter(_this, void 0, void 0, function () {
        var cpu, response;
        return tslib_1.__generator(this, function (_a) {
            cpu = os_utils_1.default.cpuCount();
            log("[NITRO]::CPU informations - ".concat(cpu));
            response = {
                numCpuPhysicalCore: cpu,
                memAvailable: 0,
            };
            resolve(response);
            return [2 /*return*/];
        });
    }); });
}
exports.default = {
    getNvidiaConfig: getNvidiaConfig,
    setNvidiaConfig: setNvidiaConfig,
    setLogger: setLogger,
    runModel: runModel,
    stopModel: stopModel,
    loadLLMModel: loadLLMModel,
    validateModelStatus: validateModelStatus,
    chatCompletion: chatCompletion,
    killSubprocess: killSubprocess,
    updateNvidiaInfo: function () { return tslib_1.__awaiter(void 0, void 0, void 0, function () { return tslib_1.__generator(this, function (_a) {
        switch (_a.label) {
            case 0: return [4 /*yield*/, (0, nvidia_1.updateNvidiaInfo)(nvidiaConfig)];
            case 1: return [2 /*return*/, _a.sent()];
        }
    }); }); },
    getCurrentNitroProcessInfo: function () { return (0, nvidia_1.getNitroProcessInfo)(subprocess); },
};
//# sourceMappingURL=index.js.map