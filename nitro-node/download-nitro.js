const path = require('path');
const download = require('download');
const stream = require('stream');

// Define nitro version to download in env variable
const NITRO_VERSION = process.env.NITRO_VERSION || '0.2.11';
// The platform OS to download nitro for
const PLATFORM = process.env.npm_config_platform || process.platform;
// The platform architecture
//const ARCH = process.env.npm_config_arch || process.arch;

const linuxVariants = {
  'linux-amd64': path.join(__dirname, 'bin', 'linux-cpu'),
  'linux-amd64-cuda-12-0': path.join(__dirname, 'bin', 'linux-cuda-12-0'),
  'linux-amd64-cuda-11-7': path.join(__dirname, 'bin', 'linux-cuda-11-7'),
}

const darwinVariants = {
  'mac-arm64': path.join(__dirname, 'bin', 'mac-arm64'),
  'mac-amd64': path.join(__dirname, 'bin', 'mac-x64'),
}

const win32Variants = {
  'win-amd64-cuda-12-0': path.join(__dirname, 'bin', 'win-cuda-12-0'),
  'win-amd64-cuda-11-7': path.join(__dirname, 'bin', 'win-cuda-11-7'),
  'win-amd64': path.join(__dirname, 'bin', 'win-cpu'),
}

// Mapping to installation variants
const variantMapping = {
  'darwin': darwinVariants,
  'linux': linuxVariants,
  'win32': win32Variants,
}

if (!(PLATFORM in variantMapping)) {
  throw Error(`Invalid platform: ${PLATFORM}`);
}
// Get download config for this platform
const variantConfig = variantMapping[PLATFORM];

// Generate download link for each tarball
const getTarUrl = (version, suffix) => `https://github.com/janhq/nitro/releases/download/v${version}/nitro-${version}-${suffix}.tar.gz`

// Report download progress
const createProgressReporter = (variant) => (stream) => stream.on(
  'downloadProgress',
  (progress) => {
    process.stdout.write(`\r[${variant}] ${progress.transferred}/${progress.total} ${Math.floor(progress.percent * 100)}%...`);
  }).on('finish', () => {
    console.log(`[${variant}] Finished downloading!`);
  })

// Download single binary
const downloadBinary = (version, suffix, filePath) => {
  const tarUrl = getTarUrl(version, suffix);
  console.log(`Downloading ${tarUrl} to ${filePath}`);
  const progressReporter = createProgressReporter(suffix);
  return progressReporter(
    download(tarUrl, filePath, {
      strip: 1,
      extract: true,
    })
  );
}

// Download the binaries
const downloadBinaries = async (version, config) => {
  await Object.entries(config).reduce(
    async (p, [k, v]) => {
      p.then(() => downloadBinary(version, k, v));
    },
    Promise.resolve(),
  );
}

// Call the download function with version and config
const run = () => {
  downloadBinaries(NITRO_VERSION, variantConfig);
}

module.exports = run;

// Run script if called directly instead of import as module
if (require.main === module) {
  run();
}
