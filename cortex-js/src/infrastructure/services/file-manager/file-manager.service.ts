import { Config } from '@/domain/config/config.interface';
import { Injectable } from '@nestjs/common';
import os from 'os';
import { join } from 'node:path';
import {
  existsSync,
  read,
  open,
  close,
  promises,
  createReadStream,
} from 'node:fs';
import { promisify } from 'util';
import yaml from 'js-yaml';
import { write } from 'fs';
import { createInterface } from 'readline';
import {
  defaultCortexCppHost,
  defaultCortexCppPort,
} from '@/infrastructure/constants/cortex';

const readFileAsync = promisify(read);
const openAsync = promisify(open);
const closeAsync = promisify(close);
const writeAsync = promisify(write);
@Injectable()
export class FileManagerService {
  private configFile = '.cortexrc';
  private cortexDirectoryName = 'cortex';
  private modelFolderName = 'models';
  private presetFolderName = 'presets';
  private extensionFoldername = 'extensions';
  private benchmarkFoldername = 'benchmark';
  private cortexCppFolderName = 'cortex-cpp';
  private cortexTelemetryFolderName = 'telemetry';

  /**
   * Get cortex configs
   * @returns the config object
   */
  async getConfig(): Promise<Config> {
    const homeDir = os.homedir();
    const configPath = join(homeDir, this.configFile);

    if (!existsSync(configPath)) {
      const config = this.defaultConfig();
      await this.createFolderIfNotExist(config.dataFolderPath);
      await this.writeConfigFile(config);
      return config;
    }

    try {
      const content = await promises.readFile(configPath, 'utf8');
      const config = yaml.load(content) as Config;
      return {
        ...this.defaultConfig(),
        ...config,
      };
    } catch (error) {
      console.warn('Error reading config file. Using default config.');
      console.warn(error);
      const config = this.defaultConfig();
      await this.createFolderIfNotExist(config.dataFolderPath);
      await this.writeConfigFile(config);
      return config;
    }
  }

  async writeConfigFile(config: Config): Promise<void> {
    const homeDir = os.homedir();
    const configPath = join(homeDir, this.configFile);

    // write config to file as yaml
    const configString = yaml.dump(config);
    await promises.writeFile(configPath, configString, 'utf8');
  }

  private async createFolderIfNotExist(dataFolderPath: string): Promise<void> {
    if (!existsSync(dataFolderPath)) {
      await promises.mkdir(dataFolderPath, { recursive: true });
    }

    const modelFolderPath = join(dataFolderPath, this.modelFolderName);
    const cortexCppFolderPath = join(dataFolderPath, this.cortexCppFolderName);
    const cortexTelemetryFolderPath = join(
      dataFolderPath,
      this.cortexTelemetryFolderName,
    );
    if (!existsSync(modelFolderPath)) {
      await promises.mkdir(modelFolderPath, { recursive: true });
    }
    if (!existsSync(cortexCppFolderPath)) {
      await promises.mkdir(cortexCppFolderPath, { recursive: true });
    }
    if (!existsSync(cortexTelemetryFolderPath)) {
      await promises.mkdir(cortexTelemetryFolderPath, { recursive: true });
    }
  }

  private defaultConfig(): Config {
    // default will store at home directory
    const homeDir = os.homedir();
    const dataFolderPath = join(homeDir, this.cortexDirectoryName);

    return {
      dataFolderPath,
      initialized: false,
      cortexCppHost: defaultCortexCppHost,
      cortexCppPort: defaultCortexCppPort,
    };
  }

  /**
   * Get the app data folder path
   * Usually it is located at the home directory > cortex
   * @returns the path to the data folder
   */
  async getDataFolderPath(): Promise<string> {
    const config = await this.getConfig();
    return config.dataFolderPath;
  }

  async getLastLine(
    filePath: string,
  ): Promise<{ data: any; position: number }> {
    try {
      const fileDescriptor = await openAsync(filePath, 'a+');
      const stats = await promises.stat(filePath);
      const bufferSize = 1024 * 5; // 5KB
      const buffer = Buffer.alloc(bufferSize);
      const filePosition = stats.size;
      let lastPosition = filePosition;
      let data = '';
      let newLineIndex = -1;

      async function readPreviousChunk() {
        const readPosition = Math.max(0, lastPosition - bufferSize);
        const chunkSize = Math.min(bufferSize, lastPosition);

        const { bytesRead } = await readFileAsync(
          fileDescriptor,
          buffer,
          0,
          chunkSize,
          readPosition,
        );
        data = buffer.slice(0, bytesRead).toString() + data;
        lastPosition -= bufferSize;
        newLineIndex = data.lastIndexOf('\n');
        if (newLineIndex === -1 && lastPosition > 0) {
          return readPreviousChunk();
        } else {
          const lastLine = data.slice(newLineIndex + 1).trim();
          await closeAsync(fileDescriptor);
          return {
            data: lastLine,
            position: readPosition + newLineIndex + 1,
          };
        }
      }

      return await readPreviousChunk();
    } catch (err) {
      //todo: add log level then log error
      throw err;
    }
  }
  async modifyLine(filePath: string, modifiedLine: any, position: number) {
    try {
      const fd = await openAsync(filePath, 'r+');
      const buffer = Buffer.from(modifiedLine, 'utf8');
      await writeAsync(fd, buffer, 0, buffer.length, position);
      await closeAsync(fd);
    } catch (err) {
      //todo: add log level then log error
      throw err;
    }
  }

  async append(filePath: string, data: any) {
    try {
      const stats = await promises.stat(filePath);
      return await promises.appendFile(
        filePath,
        stats.size === 0 ? data : `\n${data}`,
        {
          encoding: 'utf8',
          flag: 'a+',
        },
      );
    } catch (err) {
      throw err;
    }
  }
  readLines(filePath: string, callback: (line: string) => void) {
    const fileStream = createReadStream(filePath);
    const rl = createInterface({
      input: fileStream,
      crlfDelay: Infinity,
    });
    rl.on('line', callback);
  }

  /**
   * Get the models data folder path
   * Usually it is located at the home directory > cortex > models
   * @returns the path to the models folder
   */
  async getModelsPath(): Promise<string> {
    const dataFolderPath = await this.getDataFolderPath();
    return join(dataFolderPath, this.modelFolderName);
  }

  /**
   * Get the presets data folder path
   * Usually it is located at the home directory > cortex > presets
   * @returns the path to the presets folder
   */
  async getPresetsPath(): Promise<string> {
    const dataFolderPath = await this.getDataFolderPath();
    return join(dataFolderPath, this.presetFolderName);
  }

  /**
   * Get the extensions data folder path
   * Usually it is located at the home directory > cortex > extensions
   * @returns the path to the extensions folder
   */
  async getExtensionsPath(): Promise<string> {
    const dataFolderPath = await this.getDataFolderPath();
    return join(dataFolderPath, this.extensionFoldername);
  }

  /**
   * Get the benchmark folder path
   * Usually it is located at the home directory > cortex > extensions
   * @returns the path to the benchmark folder
   */
  async getBenchmarkPath(): Promise<string> {
    const dataFolderPath = await this.getDataFolderPath();
    return join(dataFolderPath, this.benchmarkFoldername);
  }

  /**
   * Get Cortex CPP engines folder path
   * @returns the path to the cortex engines folder
   */
  async getCortexCppEnginePath(): Promise<string> {
    return join(await this.getDataFolderPath(), 'cortex-cpp', 'engines');
  }

  async createFolderIfNotExistInDataFolder(folderName: string): Promise<void> {
    const dataFolderPath = await this.getDataFolderPath();
    const folderPath = join(dataFolderPath, folderName);
    if (!existsSync(folderPath)) {
      await promises.mkdir(folderPath, { recursive: true });
    }
  }
}
