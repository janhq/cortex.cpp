import fs from "node:fs";
import path from "node:path";
import download from "download";
import { Duplex } from "node:stream";

// Define nitro version to download in env variable
const NITRO_VERSION = process.env.NITRO_VERSION || "0.2.11";
// The platform OS to download nitro for
const PLATFORM = process.env.npm_config_platform || process.platform;
// The platform architecture
//const ARCH = process.env.npm_config_arch || process.arch;

const linuxVariants = {
  "linux-amd64": "linux-cpu",
  "linux-amd64-cuda-12-0": "linux-cuda-12-0",
  "linux-amd64-cuda-11-7": "linux-cuda-11-7",
};

const darwinVariants = {
  "mac-arm64": "mac-arm64",
  "mac-amd64": "mac-x64",
};

const win32Variants = {
  "win-amd64-cuda-12-0": "win-cuda-12-0",
  "win-amd64-cuda-11-7": "win-cuda-11-7",
  "win-amd64": "win-cpu",
};

// Mapping to installation variants
const variantMapping: Record<string, Record<string, string>> = {
  darwin: darwinVariants,
  linux: linuxVariants,
  win32: win32Variants,
};

if (!(PLATFORM in variantMapping)) {
  throw Error(`Invalid platform: ${PLATFORM}`);
}
// Get download config for this platform
const variantConfig: Record<string, string> = variantMapping[PLATFORM];

// Generate download link for each tarball
const getTarUrl = (version: string, suffix: string) =>
  `https://github.com/janhq/nitro/releases/download/v${version}/nitro-${version}-${suffix}.tar.gz`;

// Report download progress
const createProgressReporter =
  (variant: string) => (stream: Promise<Buffer> & Duplex) =>
    stream
      .on(
        "downloadProgress",
        (progress: { transferred: any; total: any; percent: number }) => {
          // Print and update progress on a single line of terminal
          process.stdout.write(
            `\r\x1b[K[${variant}] ${progress.transferred}/${progress.total} ${Math.floor(progress.percent * 100)}%...`,
          );
        },
      )
      .on("end", () => {
        // Jump to new line to log next message
        console.log();
        console.log(`[${variant}] Finished downloading!`);
      });

// Download single binary
const downloadBinary = async (
  version: string,
  suffix: string,
  destDirPath: string,
) => {
  const tarUrl = getTarUrl(version, suffix);
  console.log(`Downloading ${tarUrl} to ${destDirPath}`);
  const progressReporter = createProgressReporter(suffix);
  await progressReporter(
    download(tarUrl, destDirPath, {
      strip: 1,
      extract: true,
    }),
  );
  // Set mode of downloaded binaries to executable
  (fs.readdirSync(destDirPath, { recursive: true }) as string[])
    .filter(
      (fname) =>
        fname.includes("nitro") &&
        fs.lstatSync(path.join(destDirPath, fname)).isFile(),
    )
    .forEach((nitroBinary) => {
      const absPath = path.join(destDirPath, nitroBinary);
      fs.chmodSync(
        absPath,
        fs.constants.S_IRWXU | fs.constants.S_IRWXG | fs.constants.S_IRWXO,
      );
    });
};

// Download the binaries
const downloadBinaries = (
  version: string,
  config: Record<string, string>,
  absBinDirPath: string,
) => {
  return Object.entries(config).reduce(
    (p: Promise<any>, [k, v]) =>
      p.then(() => downloadBinary(version, k, path.join(absBinDirPath, v))),
    Promise.resolve(),
  );
};

// Check for a files with nitro in name in the corresponding directory
const verifyDownloadedBinaries = (absBinDirPath: string) => {
  // Check all the paths in variantConfig for a file with nitro in its name
  return Object.values(variantConfig).every((binDirVariant: string) => {
    try {
      const dirToCheck = path.join(absBinDirPath, binDirVariant);
      const pathToCheck = path.join(dirToCheck, "nitro");
      return (fs.readdirSync(dirToCheck, { recursive: true }) as string[]).some(
        (fname) => {
          const fullPath = path.join(dirToCheck, fname);
          return fullPath.startsWith(pathToCheck);
        },
      );
    } catch (_e: any) {
      return false;
    }
  });
};

// Call the download function with version and config
const downloadNitro = async (
  absBinDirPath: string = path.join(__dirname, "..", '..', "bin"),
) => {
  // Return early without downloading if nitro binaries are already downloaded
  if (verifyDownloadedBinaries(absBinDirPath)) {
    //console.log("Nitro binaries are already downloaded!");
    return;
  }
  return await downloadBinaries(NITRO_VERSION, variantConfig, absBinDirPath);
};

export default downloadNitro;

// Run script if called directly instead of import as module
if (require.main === module) {
  downloadNitro();
}
