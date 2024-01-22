const https = require('https');
const url = require('url');
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');
const EventEmitter = require('events');

// Define nitro version to download in this file
const VERSION_TXT = path.join('.', 'bin', 'version.txt');
// Read nitro version
const NITRO_VERSION = fs.readFileSync(VERSION_TXT, 'utf8').trim();
// The platform OS to download nitro for
const PLATFORM = process.env.npm_config_platform || process.platform;
// The platform architecture
//const ARCH = process.env.npm_config_arch || process.arch;

const linuxVariants = {
  'linux-amd64': path.join('.', 'bin', 'linux-cpu'),
  'linux-amd64-cuda-12-0': path.join('.', 'bin', 'linux-cuda-12-0', 'nitro'),
  'linux-amd64-cuda-11-7': path.join('.', 'bin', 'linux-cuda-11-7', 'nitro'),
}

const darwinVariants = {
  'mac-arm64': path.join('.', 'bin', 'mac-arm64', 'nitro'),
  'mac-amd64': path.join('.', 'bin', 'mac-x64', 'nitro'),
}

const win32Variants = {
  'win-amd64-cuda-12-0': path.join('.', 'bin', 'win-cuda-12-0', 'nitro.exe'),
  'win-amd64-cuda-11-7': path.join('.', 'bin', 'win-cuda-11-7', 'nitro.exe'),
  'win-amd64': path.join('.', 'bin', 'win-cpu', 'nitro.exe'),
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

// Create filestream to write the binary
const createOutputFileStream = async (filePath) => {
  // Create immediate directories if they do not exist
  const dir = path.dirname(filePath);
  const createDir = await fs.mkdirSync(dir, { recursive: true });
  if (createDir) console.log(`created ${createDir}`);
  return fs.createWriteStream(filePath);
}

// Extract tarball and write to file from the response content
const writeToFile = (fileStream, variant = '?') => (response) => {
  const totalLength = parseInt(response.headers['content-length'], 10);
  console.log(`[${variant}] Need to download ${totalLength} bytes...`);

  // TODO: promises in for loop does not actually wait so output will be gobbled
  //let currentLength = 0;
  //response.on('data', (chunk) => {
  //  currentLength += chunk.length;
  //  const percentage = Math.floor(100.0 * currentLength / totalLength);
  //  if (percentage % 10) {
  //    process.stdout.write(`\r[${variant}] ${percentage}%...`);
  //  }
  //});

  response.on('end', () => {
    console.log(`[${variant}] Finished downloading!`);
  });

  return response.pipe(zlib.createUnzip()).pipe(fileStream);
}

const promisifyWriter = (writer) => {
  const eventEmitter = new EventEmitter();
  return ({
    p: new Promise((resolve, reject) => {
      eventEmitter.on('end', resolve);
      eventEmitter.on('error', reject);
    }),
    callback(response) {
      writer(response).on('error', (err) => eventEmitter.emit('error', err)).on('end', () => eventEmitter.emit('end'));
    }
  });
}

// Download single binary
const downloadBinary = async (version, suffix, filePath) => {
  const tarUrl = getTarUrl(version, suffix);
  console.log(`Downloading ${tarUrl} to ${filePath}`);
  const fileStream = await createOutputFileStream(filePath);
  const waitable = promisifyWriter(writeToFile(fileStream, suffix));
  const writer = waitable.callback;
  https.get(tarUrl, (response) => {
    if (response.statusCode > 300 && response.statusCode < 400 && response.headers.location) {
      if (new URL(response.headers.location).hostname) {
        https.get(response.headers.location, writer);
      } else {
        https.get(
          url.resolve(
            new URL(tarUrl).hostname,
            response.headers.location
          ),
          writer
        );
      }
    } else {
      writer(response);
    }
  });
  await waitable.p;
}

// Download the binaries
const downloadBinaries = async (version, config) => {
  await Promise.all(Object.entries(config).map(async ([k, v]) => {
    await downloadBinary(version, k, v);
  }));
}

// Call the download function with version and config
downloadBinaries(NITRO_VERSION, variantConfig);
