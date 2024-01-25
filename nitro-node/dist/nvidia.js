"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.updateGpuInfo = exports.updateCudaExistence = exports.checkFileExistenceInPaths = exports.updateNvidiaDriverInfo = exports.getNitroProcessInfo = exports.updateNvidiaInfo = void 0;
var tslib_1 = require("tslib");
var node_fs_1 = require("node:fs");
var node_child_process_1 = require("node:child_process");
var node_path_1 = tslib_1.__importDefault(require("node:path"));
/**
 * Current nitro process
 */
var nitroProcessInfo = undefined;
/**
 * This will retrive GPU informations and persist settings.json
 * Will be called when the extension is loaded to turn on GPU acceleration if supported
 */
function updateNvidiaInfo(nvidiaSettings) {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        return tslib_1.__generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    if (!(process.platform !== "darwin")) return [3 /*break*/, 2];
                    return [4 /*yield*/, Promise.all([
                            updateNvidiaDriverInfo(nvidiaSettings),
                            updateCudaExistence(nvidiaSettings),
                            updateGpuInfo(nvidiaSettings),
                        ])];
                case 1:
                    _a.sent();
                    _a.label = 2;
                case 2: return [2 /*return*/];
            }
        });
    });
}
exports.updateNvidiaInfo = updateNvidiaInfo;
/**
 * Retrieve current nitro process
 */
var getNitroProcessInfo = function (subprocess) {
    nitroProcessInfo = {
        isRunning: subprocess != null,
    };
    return nitroProcessInfo;
};
exports.getNitroProcessInfo = getNitroProcessInfo;
/**
 * Validate nvidia and cuda for linux and windows
 */
function updateNvidiaDriverInfo(nvidiaSettings) {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        return tslib_1.__generator(this, function (_a) {
            (0, node_child_process_1.exec)("nvidia-smi --query-gpu=driver_version --format=csv,noheader", function (error, stdout) {
                if (!error) {
                    var firstLine = stdout.split("\n")[0].trim();
                    nvidiaSettings["nvidia_driver"].exist = true;
                    nvidiaSettings["nvidia_driver"].version = firstLine;
                }
                else {
                    nvidiaSettings["nvidia_driver"].exist = false;
                }
            });
            return [2 /*return*/];
        });
    });
}
exports.updateNvidiaDriverInfo = updateNvidiaDriverInfo;
/**
 * Check if file exists in paths
 */
function checkFileExistenceInPaths(file, paths) {
    return paths.some(function (p) { return (0, node_fs_1.existsSync)(node_path_1.default.join(p, file)); });
}
exports.checkFileExistenceInPaths = checkFileExistenceInPaths;
/**
 * Validate cuda for linux and windows
 */
function updateCudaExistence(nvidiaSettings) {
    var filesCuda12;
    var filesCuda11;
    var paths;
    var cudaVersion = "";
    if (process.platform === "win32") {
        filesCuda12 = ["cublas64_12.dll", "cudart64_12.dll", "cublasLt64_12.dll"];
        filesCuda11 = ["cublas64_11.dll", "cudart64_11.dll", "cublasLt64_11.dll"];
        paths = process.env.PATH ? process.env.PATH.split(node_path_1.default.delimiter) : [];
    }
    else {
        filesCuda12 = ["libcudart.so.12", "libcublas.so.12", "libcublasLt.so.12"];
        filesCuda11 = ["libcudart.so.11.0", "libcublas.so.11", "libcublasLt.so.11"];
        paths = process.env.LD_LIBRARY_PATH
            ? process.env.LD_LIBRARY_PATH.split(node_path_1.default.delimiter)
            : [];
        paths.push("/usr/lib/x86_64-linux-gnu/");
    }
    var cudaExists = filesCuda12.every(function (file) { return (0, node_fs_1.existsSync)(file) || checkFileExistenceInPaths(file, paths); });
    if (!cudaExists) {
        cudaExists = filesCuda11.every(function (file) { return (0, node_fs_1.existsSync)(file) || checkFileExistenceInPaths(file, paths); });
        if (cudaExists) {
            cudaVersion = "11";
        }
    }
    else {
        cudaVersion = "12";
    }
    nvidiaSettings["cuda"].exist = cudaExists;
    nvidiaSettings["cuda"].version = cudaVersion;
    if (cudaExists) {
        nvidiaSettings.run_mode = "gpu";
    }
}
exports.updateCudaExistence = updateCudaExistence;
/**
 * Get GPU information
 */
function updateGpuInfo(nvidiaSettings) {
    return tslib_1.__awaiter(this, void 0, void 0, function () {
        return tslib_1.__generator(this, function (_a) {
            (0, node_child_process_1.exec)("nvidia-smi --query-gpu=index,memory.total --format=csv,noheader,nounits", function (error, stdout) {
                if (!error) {
                    // Get GPU info and gpu has higher memory first
                    var highestVram_1 = 0;
                    var highestVramId_1 = "0";
                    var gpus = stdout
                        .trim()
                        .split("\n")
                        .map(function (line) {
                        var _a = line.split(", "), id = _a[0], vram = _a[1];
                        vram = vram.replace(/\r/g, "");
                        if (parseFloat(vram) > highestVram_1) {
                            highestVram_1 = parseFloat(vram);
                            highestVramId_1 = id;
                        }
                        return { id: id, vram: vram };
                    });
                    nvidiaSettings["gpus"] = gpus;
                    nvidiaSettings["gpu_highest_vram"] = highestVramId_1;
                }
                else {
                    nvidiaSettings["gpus"] = [];
                }
            });
            return [2 /*return*/];
        });
    });
}
exports.updateGpuInfo = updateGpuInfo;
//# sourceMappingURL=nvidia.js.map