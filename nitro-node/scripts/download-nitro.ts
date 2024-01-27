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
  "linux-amd64": path.normalize(path.join(__dirname, "..", "bin", "linux-cpu")),
  "linux-amd64-cuda-12-0": path.normalize(
    path.join(__dirname, "..", "bin", "linux-cuda-12-0"),
  ),
  "linux-amd64-cuda-11-7": path.normalize(
    path.join(__dirname, "..", "bin", "linux-cuda-11-7"),
  ),
};

const darwinVariants = {
  "mac-arm64": path.normalize(path.join(__dirname, "..", "bin", "mac-arm64")),
  "mac-amd64": path.normalize(path.join(__dirname, "..", "bin", "mac-x64")),
};

const win32Variants = {
  "win-amd64-cuda-12-0": path.normalize(
    path.join(__dirname, "..", "bin", "win-cuda-12-0"),
  ),
  "win-amd64-cuda-11-7": path.normalize(
    path.join(__dirname, "..", "bin", "win-cuda-11-7"),
  ),
  "win-amd64": path.normalize(path.join(__dirname, "..", "bin", "win-cpu")),
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
    }));
  // Set mode of downloaded binaries to executable
  (fs.readdirSync(destDirPath, { recursive: true }) as string[])
    .filter((fname) => fs.lstatSync(path.join(destDirPath, fname)).isFile())
    .filter((fname_1) => fname_1.includes("nitro"))
    .forEach((nitroBinary) => {
      const absPath = path.join(destDirPath, nitroBinary);
      fs.chmodSync(
        absPath,
        fs.constants.S_IRWXU | fs.constants.S_IRWXG | fs.constants.S_IRWXO);
    });
};

// Download the binaries
const downloadBinaries = (version: string, config: Record<string, string>) => {
  return Object.entries(config).reduce(
    (p: Promise<any>, [k, v]) => p.then(() => downloadBinary(version, k, v)),
    Promise.resolve(),
  );
};

// Check for a files with nitro in name in the corresponding directory
const verifyDownloadedBinaries = () => {
  // Check all the paths in variantConfig for a file with nitro in its name
  return Object.values(variantConfig).every((binDirVariant: string) => {
    try {
      const pathToCheck = path.join(binDirVariant, "nitro");
      return (
        fs.readdirSync(binDirVariant, { recursive: true }) as string[]
      ).some((fname) => {
        const fullPath = path.join(binDirVariant, fname);
        return fullPath.startsWith(pathToCheck);
      });
    } catch (_e: any) {
      return false;
    }
  });
};

// Call the download function with version and config
const downloadNitro = async () => {
  // Return early without downloading if nitro binaries are already downloaded
  if (verifyDownloadedBinaries()) {
    //console.log("Nitro binaries are already downloaded!");
    return;
  }
  return await downloadBinaries(NITRO_VERSION, variantConfig);
};

export default downloadNitro;

// Run script if called directly instead of import as module
if (require.main === module) {
  downloadNitro();
}
