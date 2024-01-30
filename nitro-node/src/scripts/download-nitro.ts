import fs from "node:fs";
import path from "node:path";
import download from "download";
import { Duplex } from "node:stream";

// Define nitro version to download in env variable ("latest" or tag "v1.2.3")
const NITRO_VERSION = process.env.NITRO_VERSION || "latest";
// The platform OS to download nitro for
const PLATFORM = process.env.npm_config_platform || process.platform;
// The platform architecture
//const ARCH = process.env.npm_config_arch || process.arch;

const releaseUrlPrefixVariants = {
  latest: "https://api.github.com/repos/janhq/nitro/releases/",
  tag: "https://api.github.com/repos/janhq/nitro/releases/tags/",
};

const getReleaseInfo = async (taggedVersion: string): Promise<any> => {
  const releaseUrlPrefix =
    taggedVersion === "latest"
      ? releaseUrlPrefixVariants.latest
      : taggedVersion.match(/^v?\d+\.\d+\.\d+$/gi)
        ? releaseUrlPrefixVariants.tag
        : undefined;
  if (!releaseUrlPrefix) throw Error(`Invalid version: ${taggedVersion}`);
  const url = `${releaseUrlPrefix}${taggedVersion}`;
  console.log(`[Getting release info] ${url}`);
  const response = await fetch(url, {
    headers: {
      Accept: "application/vnd.github+json",
      "X-GitHub-Api-Version": "2022-11-28",
    },
  });
  if (!response.ok)
    throw Error(`Failed to fetch release info: ${response.status}`);
  return await response.json();
};

const extractDownloadInfo = async (
  /* releaseInfo */
  {
    html_url,
    tag_name,
    name,
    prerelease,
    assets,
  }: {
    html_url: string;
    tag_name: string;
    name: string;
    prerelease: boolean;
    assets: {
      name: string;
      size: number;
      browser_download_url: string;
    }[];
  },
  /* variants for filter for in format {suffix: dir} */
  suffixVariants: Record<string, string>,
): Promise<Record<string, { url: string; dir: string }>> => {
  console.log(`[Release URL][prerelease=${prerelease}] ${html_url}`);
  const assetNames = assets.map(({ name }: { name: string }) => name);
  const assetsToDownloads = Object.entries(suffixVariants).reduce(
    (
      dict: Record<string, { url: string; dir: string }>,
      [suffix, dir]: [string, string],
    ) => {
      // Skip if suffix is not in asset names
      if (!assetNames.includes(`nitro-${name}-${suffix}.tar.gz`)) return dict;
      // Else add the download url
      dict[suffix] = {
        url: `https://github.com/janhq/nitro/releases/download/${tag_name}/nitro-${name}-${suffix}.tar.gz`,
        dir,
      };
      return dict;
    },
    {},
  );
  // If empty then no assets were found
  if (!Object.keys(assetsToDownloads).length)
    throw Error("Failed to find any asset to download");
  // Return the dict of download info
  return assetsToDownloads;
};

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
  suffix: string,
  { url, dir }: { url: string; dir: string },
) => {
  console.log(`Downloading ${url} to ${dir}...`);
  const progressReporter = createProgressReporter(suffix);
  await progressReporter(
    download(url, dir, {
      strip: 1,
      extract: true,
    }),
  );
  // Set mode of downloaded binaries to executable
  (fs.readdirSync(dir, { recursive: true }) as string[])
    .filter(
      (fname) =>
        fname.includes("nitro") && fs.lstatSync(path.join(dir, fname)).isFile(),
    )
    .forEach((nitroBinary) => {
      const absPath = path.join(dir, nitroBinary);
      // Set mode executable for nitro binary
      fs.chmodSync(
        absPath,
        fs.constants.S_IRWXU | fs.constants.S_IRWXG | fs.constants.S_IRWXO,
      );
    });
};

// Download the binaries
const downloadBinaries = (
  /* downloadInfo */
  downloadInfo: Record<string, { url: string; dir: string }>,
  /* The absolute path to the directory where binaries will be downloaded */
  absBinDirPath: string,
) => {
  return Object.entries(downloadInfo).reduce(
    (
      p: Promise<any>,
      [suffix, { url, dir }]: [string, { url: string; dir: string }],
    ) =>
      p.then(() =>
        downloadBinary(suffix, { url, dir: path.join(absBinDirPath, dir) }),
      ),
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
// Call the download function with version and config
export const downloadNitro = async (absBinDirPath: string) => {
  // Return early without downloading if nitro binaries are already downloaded
  if (verifyDownloadedBinaries(absBinDirPath)) {
    //console.log("Nitro binaries are already downloaded!");
    return;
  }
  const releaseInfo = await getReleaseInfo(NITRO_VERSION);
  const downloadInfo = await extractDownloadInfo(releaseInfo, variantConfig);
  return await downloadBinaries(downloadInfo, absBinDirPath);
};

// Run script if called directly instead of import as module
if (require.main === module) {
  // Assume calling the source typescript files
  // bin path will be relative to the source code root
  downloadNitro(path.join(__dirname, "..", "..", "bin"));
}
