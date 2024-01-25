'use strict';

var path = require('node:path');
var download = require('download');

function _interopDefaultLegacy (e) { return e && typeof e === 'object' && 'default' in e ? e : { 'default': e }; }

var path__default = /*#__PURE__*/_interopDefaultLegacy(path);
var download__default = /*#__PURE__*/_interopDefaultLegacy(download);

// Define nitro version to download in env variable
var NITRO_VERSION = process.env.NITRO_VERSION || "0.2.11";
// The platform OS to download nitro for
var PLATFORM = process.env.npm_config_platform || process.platform;
// The platform architecture
//const ARCH = process.env.npm_config_arch || process.arch;
var linuxVariants = {
    "linux-amd64": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "linux-cpu")),
    "linux-amd64-cuda-12-0": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "linux-cuda-12-0")),
    "linux-amd64-cuda-11-7": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "linux-cuda-11-7")),
};
var darwinVariants = {
    "mac-arm64": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "mac-arm64")),
    "mac-amd64": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "mac-x64")),
};
var win32Variants = {
    "win-amd64-cuda-12-0": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "win-cuda-12-0")),
    "win-amd64-cuda-11-7": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "win-cuda-11-7")),
    "win-amd64": path__default["default"].normalize(path__default["default"].join(__dirname, "..", "bin", "win-cpu")),
};
// Mapping to installation variants
var variantMapping = {
    darwin: darwinVariants,
    linux: linuxVariants,
    win32: win32Variants,
};
if (!(PLATFORM in variantMapping)) {
    throw Error("Invalid platform: ".concat(PLATFORM));
}
// Get download config for this platform
var variantConfig = variantMapping[PLATFORM];
// Generate download link for each tarball
var getTarUrl = function (version, suffix) {
    return "https://github.com/janhq/nitro/releases/download/v".concat(version, "/nitro-").concat(version, "-").concat(suffix, ".tar.gz");
};
// Report download progress
var createProgressReporter = function (variant) { return function (stream) {
    return stream
        .on("downloadProgress", function (progress) {
        // Print and update progress on a single line of terminal
        process.stdout.write("\r\u001B[K[".concat(variant, "] ").concat(progress.transferred, "/").concat(progress.total, " ").concat(Math.floor(progress.percent * 100), "%..."));
    })
        .on("end", function () {
        // Jump to new line to log next message
        console.log();
        console.log("[".concat(variant, "] Finished downloading!"));
    });
}; };
// Download single binary
var downloadBinary = function (version, suffix, filePath) {
    var tarUrl = getTarUrl(version, suffix);
    console.log("Downloading ".concat(tarUrl, " to ").concat(filePath));
    var progressReporter = createProgressReporter(suffix);
    return progressReporter(download__default["default"](tarUrl, filePath, {
        strip: 1,
        extract: true,
    }));
};
// Download the binaries
var downloadBinaries = function (version, config) {
    return Object.entries(config).reduce(function (p, _a) {
        var k = _a[0], v = _a[1];
        return p.then(function () { return downloadBinary(version, k, v); });
    }, Promise.resolve());
};
// Call the download function with version and config
var downloadNitro = function () {
    downloadBinaries(NITRO_VERSION, variantConfig);
};
// Run script if called directly instead of import as module
if (require.main === module) {
    downloadNitro();
}

module.exports = downloadNitro;
//# sourceMappingURL=download-nitro.cjs.js.map
