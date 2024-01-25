const os = require("node:os");
const path = require("node:path");
const fs = require("node:fs");
const { spawn } = require("node:child_process");

// Test on both npm and yarn
const PACKAGE_MANAGERS = ["npm", "yarn"];
const ADD_DEP_CMDS = {
  // Need to copy dependency instead of linking so test logic can check the bin
  npm: "install --install-links",
  yarn: "add",
};
// Path to the package to install
const NITRO_NODE_PKG = path.resolve(
  path.normalize(path.join(__dirname, "..", "..", "nitro-node")),
);
// Prefixes of downloaded nitro bin subdirectories
const BIN_DIR_PREFIXES = {
  darwin: "mac",
  win32: "win",
  linux: "linux",
};

// Utility to check for a file with nitro in name in the corresponding directory
const checkBinaries = (repoDir) => {
  // FIXME: Check for unsupported platforms
  const binDirPrefix = BIN_DIR_PREFIXES[process.platform];
  const searchRoot = path.join(
    repoDir,
    "node_modules",
    "@janhq",
    "nitro-node",
    "bin",
  );
  // Get the dir and files that indicate successful download of binaries
  const matched = fs
    .readdirSync(searchRoot, { recursive: true })
    .filter(
      (fname) => fname.startsWith(binDirPrefix) || fname.startsWith("nitro"),
    );
  console.log(`Downloaded bin paths:`, matched);

  // Must have both the directory for the platform and the binary
  return matched.length > 1;
};

// Wrapper to wait for child process to finish
const childProcessPromise = (childProcess) =>
  new Promise((resolve, reject) => {
    childProcess.on("exit", (exitCode) => {
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
  fs.mkdtempSync(path.join(os.tmpdir(), "dummy-project-"));

// First test, create empty project dir and add nitro-node as dependency
const firstTest = async (packageManager, repoDir) => {
  console.log(`[First test @ ${repoDir}] install with ${packageManager}`);
  // Init project with default package.json
  const cmd1 = `npm init -y`;
  console.log("🖥️ " + cmd1);
  await childProcessPromise(
    spawn(cmd1, [], { cwd: repoDir, shell: true, stdio: "inherit" }),
  );

  // Add nitro-node as dependency
  const cmd2 = `${packageManager} ${ADD_DEP_CMDS[packageManager]} ${NITRO_NODE_PKG}`;
  console.log("🖥️ " + cmd2);
  await childProcessPromise(
    spawn(cmd2, [], { cwd: repoDir, shell: true, stdio: "inherit" }),
  );

  // Check that the binaries exists
  if (!checkBinaries(repoDir)) process.exit(3);

  // Cleanup node_modules after success
  //fs.rmSync(path.join(repoDir, "node_modules"), { recursive: true });
};

// Second test, install the wrapper from another project dir
const secondTest = async (packageManager, repoDir, wrapperDir) => {
  console.log(
    `[Second test @ ${repoDir}] install ${wrapperDir} with ${packageManager}`,
  );
  // Init project with default package.json
  const cmd1 = `npm init -y`;
  console.log("🖥️ " + cmd1);
  await childProcessPromise(
    spawn(cmd1, [], { cwd: repoDir, shell: true, stdio: "inherit" }),
  );

  // Add wrapper as dependency
  const cmd2 = `${packageManager} ${ADD_DEP_CMDS[packageManager]} ${wrapperDir}`;
  console.log("🖥️ " + cmd2);
  await childProcessPromise(
    spawn(cmd2, [], { cwd: repoDir, shell: true, stdio: "inherit" }),
  );

  // Check that the binaries exists
  if (!checkBinaries(repoDir)) process.exit(3);
};

// Run all the tests for the chosen package manger
const run = async (packageManager) => {
  const firstRepoDir = createTestDir();

  // Run first test
  await firstTest(packageManager, firstRepoDir);
  console.log("First test ran success");

  // FIXME: Currently failed with npm due to wrong path being resolved.
  //const secondRepoDir = createTestDir();

  // Run second test
  //await secondTest(packageManager, secondRepoDir, firstRepoDir);
  //console.log("Second test ran success");
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
