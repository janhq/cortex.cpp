const { promises } = require('node:fs');
const os = require('os');
const fs = require('fs');
const path = require('path');
const yaml = require('js-yaml');
const constants = require('./dist/src/infrastructure/constants/cortex.js')

const uninstall = async () => {
  const cortexConfigPath = path.join(os.homedir(), '.cortexrc');
  if (fs.existsSync(cortexConfigPath)) {
    const content = await promises.readFile(cortexConfigPath, 'utf8');
    const config = yaml.load(content);
    if(!config) {
      return;
    }   
    const { dataFolderPath, cortexCppHost, cortexCppPort, apiServerHost, apiServerPort } = config;

    await fetch(constants.CORTEX_CPP_PROCESS_DESTROY_URL(cortexCppHost, cortexCppPort), 
      { method: 'DELETE' })
    .catch(() => {})
    await fetch(constants.CORTEX_JS_SYSTEM_URL(apiServerHost, apiServerPort), 
      { method: 'DELETE' })
    .catch(() => {})
    // remove all data in data folder path except models
    const modelsFolderPath = path.join(dataFolderPath, 'models');
    const files = fs.readdirSync(dataFolderPath);
    for (const file of files) {
      const fileStat = fs.statSync(path.join(dataFolderPath, file));
      if (file !== 'models') {
        if (fileStat.isDirectory()) {
          fs.rmSync(path.join(dataFolderPath, file), { recursive: true });
        } else {
          fs.unlinkSync(path.join(dataFolderPath, file));
        }
      }
    }
  }
};

uninstall();
