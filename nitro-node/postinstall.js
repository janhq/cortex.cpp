// Only run if this package is installed as dependency
if (process.env.INIT_CWD === process.cwd()) process.exit();

const downloadNitro = require(`${__dirname}/scripts/download-nitro`);
downloadNitro();
