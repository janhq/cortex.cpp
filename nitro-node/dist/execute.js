"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.executableNitroFile = void 0;
var tslib_1 = require("tslib");
var node_path_1 = tslib_1.__importDefault(require("node:path"));
/**
 * Find which executable file to run based on the current platform.
 * @returns The name of the executable file to run.
 */
var executableNitroFile = function (nvidiaSettings) {
    var binaryFolder = node_path_1.default.join(__dirname, "..", "bin"); // Current directory by default
    var cudaVisibleDevices = "";
    var binaryName = "nitro";
    /**
     * The binary folder is different for each platform.
     */
    if (process.platform === "win32") {
        /**
         *  For Windows: win-cpu, win-cuda-11-7, win-cuda-12-0
         */
        if (nvidiaSettings["run_mode"] === "cpu") {
            binaryFolder = node_path_1.default.join(binaryFolder, "win-cpu");
        }
        else {
            if (nvidiaSettings["cuda"].version === "12") {
                binaryFolder = node_path_1.default.join(binaryFolder, "win-cuda-12-0");
            }
            else {
                binaryFolder = node_path_1.default.join(binaryFolder, "win-cuda-11-7");
            }
            cudaVisibleDevices = nvidiaSettings["gpu_highest_vram"];
        }
        binaryName = "nitro.exe";
    }
    else if (process.platform === "darwin") {
        /**
         *  For MacOS: mac-arm64 (Silicon), mac-x64 (InteL)
         */
        if (process.arch === "arm64") {
            binaryFolder = node_path_1.default.join(binaryFolder, "mac-arm64");
        }
        else {
            binaryFolder = node_path_1.default.join(binaryFolder, "mac-x64");
        }
    }
    else {
        if (nvidiaSettings["run_mode"] === "cpu") {
            binaryFolder = node_path_1.default.join(binaryFolder, "linux-cpu");
        }
        else {
            if (nvidiaSettings["cuda"].version === "12") {
                binaryFolder = node_path_1.default.join(binaryFolder, "linux-cuda-12-0");
            }
            else {
                binaryFolder = node_path_1.default.join(binaryFolder, "linux-cuda-11-7");
            }
            cudaVisibleDevices = nvidiaSettings["gpu_highest_vram"];
        }
    }
    return {
        executablePath: node_path_1.default.join(binaryFolder, binaryName),
        cudaVisibleDevices: cudaVisibleDevices,
    };
};
exports.executableNitroFile = executableNitroFile;
//# sourceMappingURL=execute.js.map