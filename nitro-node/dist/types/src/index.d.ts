/**
 * Get current Nvidia config
 * @returns {NitroNvidiaConfig} A copy of the config object
 * The returned object should be used for reading only
 * Writing to config should be via the function {@setNvidiaConfig}
 */
declare function getNvidiaConfig(): NitroNvidiaConfig;
/**
 * Set custom Nvidia config for running inference over GPU
 * @param {NitroNvidiaConfig} config The new config to apply
 */
declare function setNvidiaConfig(config: NitroNvidiaConfig): void;
/**
 * Set logger before running nitro
 * @param {NitroLogger} logger The logger to use
 */
declare function setLogger(logger: NitroLogger): void;
/**
 * Stops a Nitro subprocess.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
declare function stopModel(): Promise<NitroModelOperationResponse>;
/**
 * Initializes a Nitro subprocess to load a machine learning model.
 * @param modelFullPath - The absolute full path to model directory.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 * TODO: Should pass absolute of the model file instead of just the name - So we can modurize the module.ts to npm package
 */
declare function runModel({ modelFullPath, promptTemplate, }: NitroModelInitOptions): Promise<NitroModelOperationResponse | Error>;
/**
 * Loads a LLM model into the Nitro subprocess by sending a HTTP POST request.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
declare function loadLLMModel(settings: any): Promise<Response>;
/**
 * Run chat completion by sending a HTTP POST request and stream the response if outStream is specified
 * @param {any} request The request that is then sent to nitro
 * @param {WritableStream} outStream Optional stream that consume the response body
 * @returns {Promise<Response>} A Promise that resolves when the chat completion success, or rejects with an error if the completion fails.
 * @description If outStream is specified, the response body is consumed and cannot be used to reconstruct the data
 */
declare function chatCompletion(request: any, outStream?: WritableStream): Promise<Response>;
/**
 * Validates the status of a model.
 * @returns {Promise<NitroModelOperationResponse>} A promise that resolves to an object.
 * If the model is loaded successfully, the object is empty.
 * If the model is not loaded successfully, the object contains an error message.
 */
declare function validateModelStatus(): Promise<NitroModelOperationResponse>;
/**
 * Terminates the Nitro subprocess.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
declare function killSubprocess(): Promise<NitroModelOperationResponse>;
declare const _default: {
    getNvidiaConfig: typeof getNvidiaConfig;
    setNvidiaConfig: typeof setNvidiaConfig;
    setLogger: typeof setLogger;
    runModel: typeof runModel;
    stopModel: typeof stopModel;
    loadLLMModel: typeof loadLLMModel;
    validateModelStatus: typeof validateModelStatus;
    chatCompletion: typeof chatCompletion;
    killSubprocess: typeof killSubprocess;
    updateNvidiaInfo: () => Promise<void>;
    getCurrentNitroProcessInfo: () => import("./nvidia").NitroProcessInfo;
};
export default _default;
