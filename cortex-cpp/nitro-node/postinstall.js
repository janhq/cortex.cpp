// Only run if this package is installed as dependency
if (process.env.INIT_CWD === process.cwd()) process.exit();

const path = require("node:path");
const { downloadNitro } = require("@janhq/nitro-node/scripts");
downloadNitro(path.join(__dirname, "bin"));
