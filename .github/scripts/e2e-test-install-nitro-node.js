const os = require("node:os");
const path = require("node:path");
const fs = require("node:fs");
const { exec } = require("node:child_process");

// Test on both npm and yarn
const PACKAGE_MANAGERS = ["npm", "yarn"];
// TODO: Path to the package to install. Change to '@janhq/nitro-node' after published
const NITRO_NODE_PKG = path.normalize(
  path.join(__dirname, "..", "..", "nitro-node"),
);
const BIN_DIR_PREFIXES = {
  darwin: "mac",
  win32: "win",
  linux: "linux",
};

// Utility to check for a file with nitro in name in the corresponding directory
const checkBinaries = (repoDir) => {
  // FIXME: Check for unsupported platforms
  const binDirPrefix = BIN_DIR_PREFIXES[process.platform];
  const searchRoot = path.join(repoDir, "node_modules", "@janhq", "nitro-node");
  // Get the dir and files that indicate successful download of binaries
  const matched = fs
    .readdirSync(searchRoot, { recursive: true })
    .filter(
      (fname) => fname.startsWith(binDirPrefix) || fname.startsWith("nitro"),
    );

  // Must have both the directory for the platform and the binary
  return (
    matched.some((fname) => fname.startsWith(binDirPrefix)) &&
    matched.some((fname) => fname.startsWith(binDirPrefix))
  );
};

// Wrapper to wait for child process to finish
const childProcessPromise = (cmd) =>
  new Promise((resolve, reject) => {
    cmd.on("exit", (exitCode) => {
      const exitNum = Number(exitCode);
      if (0 == exitNum) {
        resolve();
      } else {
        reject(exitNum);
      }
    });
  });

// Create a temporary directory for testing
const createTestDir = () =>
  fs.mkdtempSync(path.join(os.tmpdir(), "dummy-project"));

// First test, create empty project dir and add nitro-node as dependency
const firstTest = async (packageManager, repoDir) => {
  console.log(`[First test @ ${repoDir}] install with ${packageManager}`);
  // Init project with default package.json
  const cmd1 = exec(`npm init -y`, { cwd: repoDir }, (err) => {
    if (err) {
      console.error(err);
      // Error at first step
      process.exit(1);
    }
  });
  await childProcessPromise(cmd1);

  // Add nitro-node as dependency
  const cmd2 = exec(
    `${packageManager} add ${NITRO_NODE_PKG}`,
    { cwd: repoDir },
    (err) => {
      if (err) {
        console.error(err);
        // Error at second step
        process.exit(2);
      }
    },
  );
  await childProcessPromise(cmd2);

  // Check that the binaries exists
  if (!checkBinaries(repoDir)) process.exit(3);

  // Cleanup node_modules after success
  fs.rmSync(path.join(repoDir, "node_modules"), { recursive: true });
};

// Second test, install the wrapper from another project dir
const secondTest = async (packageManager, repoDir, wrapperDir) => {
  console.log(
    `[Second test @ ${repoDir}] install ${wrapperDir} with ${packageManager}`,
  );
  // Init project with default package.json
  const cmd1 = exec(`npm init -y`, { cwd: repoDir }, (err) => {
    if (err) {
      console.error(err);
      // Error at first step
      process.exit(1);
    }
  });
  await childProcessPromise(cmd1);

  // Add wrapper as dependency
  const cmd2 = exec(
    `${packageManager} add ${wrapperDir}`,
    { cwd: repoDir },
    (err) => {
      if (err) {
        console.error(err);
        // Error at second step
        process.exit(2);
      }
    },
  );
  await childProcessPromise(cmd2);

  // Check that the binaries exists
  if (!checkBinaries(repoDir)) process.exit(3);
};

// Run all the tests for the chosen package manger
const run = async (packageManager) => {
  const firstRepoDir = createTestDir();
  const secondRepoDir = createTestDir();

  // Run first test
  await firstTest(packageManager, firstRepoDir);
  // Run second test
  await secondTest(packageManager, secondRepoDir, firstRepoDir);
};

// Main, run tests for npm and yarn
const main = async () => {
  await PACKAGE_MANAGERS.reduce(
    (p, pkgMng) => p.then(() => run(pkgMng)),
    Promise.resolve(),
  );
};

// Run script if called directly instead of import as module
if (require.main === module) {
  main();
}
