const { promises } = require('node:fs');
const os = require('os');
const fs = require('fs');
const path = require('path');
const yaml = require('js-yaml');

const uninstall = async () => {
    const cortexConfigPath = path.join(os.homedir(), '.cortexrc');
    if (fs.existsSync(cortexConfigPath)) {
        const content = await promises.readFile(cortexConfigPath, 'utf8');
        const config = yaml.load(content);
        if(!config) {
            return;
        }   
        const { dataFolderPath } = config;
        const modelsFolderPath = path.join(dataFolderPath, 'models');
        // remove all data in data folder path except models
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