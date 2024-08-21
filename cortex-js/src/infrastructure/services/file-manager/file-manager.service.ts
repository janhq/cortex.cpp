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
import yaml, { load } from 'js-yaml';
import { readdirSync, readFileSync, write } from 'fs';
import { createInterface } from 'readline';
import {
  defaultCortexCppHost,
  defaultCortexCppPort,
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';

const readFileAsync = promisify(read);
const openAsync = promisify(open);
const closeAsync = promisify(close);
const writeAsync = promisify(write);

@Injectable()
export class FileManagerService {
  private cortexDirectoryName = 'cortex';
  private modelFolderName = 'models';
  private presetFolderName = 'presets';
  private extensionFoldername = 'extensions';
  private benchmarkFoldername = 'benchmark';
  private cortexEnginesFolderName = 'engines';
  private cortexTelemetryFolderName = 'telemetry';
  private logFileName = 'cortex.log';
  private configProfile = process.env.CORTEX_PROFILE || 'default';
  private configPath = process.env.CORTEX_CONFIG_PATH || os.homedir();
  private customLogPath = process.env.CORTEX_LOG_PATH || '';
  /**
   * Get cortex configs
   * @returns the config object
   */
  async getConfig(dataFolderPath?: string): Promise<Config & object> {
    const configPath = join(
      this.configPath,
      this.getConfigFileName(this.configProfile),
    );
    const config = this.defaultConfig();
    const dataFolderPathUsed = dataFolderPath || config.dataFolderPath;
    if (!existsSync(configPath) || !existsSync(dataFolderPathUsed)) {
      if (!existsSync(dataFolderPathUsed)) {
        await this.createFolderIfNotExist(dataFolderPathUsed);
      }
      if (!existsSync(configPath)) {
        await this.writeConfigFile(config);
      }
      return config;
    }

    try {
      const content = await promises.readFile(configPath, 'utf8');
      const config = yaml.load(content) as Config & object;
      return {
        ...this.defaultConfig(),
        ...config,
      };
    } catch (error) {
      console.warn('Error reading config file. Using default config.');
      console.warn(error);
      await this.createFolderIfNotExist(dataFolderPathUsed);
      await this.writeConfigFile(config);
      return config;
    }
  }

  async writeConfigFile(config: Config & object): Promise<void> {
    const configPath = join(
      this.configPath,
      this.getConfigFileName(this.configProfile),
    );

    // write config to file as yaml
    if (!existsSync(configPath)) {
      await promises.writeFile(configPath, '', 'utf8');
    }
    const content = await promises.readFile(configPath, 'utf8');
    const currentConfig = yaml.load(content) as Config & object;
    const configString = yaml.dump({
      ...currentConfig,
      ...config,
    });
    await promises.writeFile(configPath, configString, 'utf8');
  }

  private async createFolderIfNotExist(dataFolderPath: string): Promise<void> {
    if (!existsSync(dataFolderPath)) {
      await promises.mkdir(dataFolderPath, { recursive: true });
    }

    const modelFolderPath = join(dataFolderPath, this.modelFolderName);
    const cortexCppFolderPath = join(
      dataFolderPath,
      this.cortexEnginesFolderName,
    );
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

  public defaultConfig(): Config {
    // default will store at home directory
    const homeDir = os.homedir();
    const dataFolderPath = join(homeDir, this.cortexDirectoryName);

    return {
      dataFolderPath,
      cortexCppHost: defaultCortexCppHost,
      cortexCppPort: defaultCortexCppPort,
      apiServerHost: defaultCortexJsHost,
      apiServerPort: defaultCortexJsPort,
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
  readLines(
    filePath: string,
    callback: (line: string) => void,
    start: number = 0,
  ) {
    const fileStream = createReadStream(filePath, {
      start,
    });
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
   * Get the preset data
   * Usually it is located at the home directory > cortex > presets > preset.yaml
   * @returns the preset data
   */
  async getPreset(preset?: string): Promise<object | undefined> {
    if (!preset) return undefined;

    const dataFolderPath = await this.getDataFolderPath();
    const presetsFolder = join(dataFolderPath, this.presetFolderName);
    if (!existsSync(presetsFolder)) return {};

    const presetFile = readdirSync(presetsFolder).find(
      (file) =>
        file.toLowerCase() === `${preset?.toLowerCase()}.yaml` ||
        file.toLowerCase() === `${preset?.toLocaleLowerCase()}.yml`,
    );
    if (!presetFile) return {};
    const presetPath = join(presetsFolder, presetFile);

    if (!preset || !existsSync(presetPath)) return {};
    return preset
      ? (load(readFileSync(join(presetPath), 'utf-8')) as object)
      : {};
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
    return join(await this.getDataFolderPath(), this.cortexEnginesFolderName);
  }

  /**
   * Get log path
   * @returns the path to the cortex engines folder
   */
  async getLogPath(): Promise<string> {
    if (this.customLogPath) {
      return this.customLogPath + `/${this.logFileName}`;
    }
    return join(await this.getDataFolderPath(), this.logFileName);
  }

  async createFolderIfNotExistInDataFolder(folderName: string): Promise<void> {
    const dataFolderPath = await this.getDataFolderPath();
    const folderPath = join(dataFolderPath, folderName);
    if (!existsSync(folderPath)) {
      await promises.mkdir(folderPath, { recursive: true });
    }
  }

  async readFile(filePath: string): Promise<string | null> {
    try {
      const isFileExist = existsSync(filePath);
      if (!isFileExist) {
        return null;
      }
      const content = await promises.readFile(filePath, {
        encoding: 'utf8',
      });
      return content;
    } catch (error) {
      throw error;
    }
  }
  async writeFile(filePath: string, data: any): Promise<void> {
    try {
      return promises.writeFile(filePath, data, {
        encoding: 'utf8',
        flag: 'w+',
      });
    } catch (error) {
      throw error;
    }
  }

  /**
   * Get the cortex server configurations
   * It is supposed to be stored in the home directory > .cortexrc
   * @returns the server configurations
   */
  getServerConfig(): { host: string; port: number } {
    const configPath = join(
      this.configPath,
      this.getConfigFileName(this.configProfile),
    );
    let config = this.defaultConfig();
    try {
      const content = readFileSync(configPath, 'utf8');
      const currentConfig = (yaml.load(content) as Config & object) ?? {};
      if (currentConfig) {
        config = currentConfig;
      }
    } catch {}
    return {
      host: config.apiServerHost ?? '127.0.0.1',
      port: config.apiServerPort ?? 1337,
    };
  }

  public setConfigProfile(profile: string) {
    this.configProfile = profile;
  }

  public getConfigProfile() {
    return this.configProfile;
  }
  public profileConfigExists(profile: string): boolean {
    const configPath = join(this.configPath, this.getConfigFileName(profile));
    try {
      const content = readFileSync(configPath, 'utf8');
      const config = yaml.load(content) as Config & object;
      return !!config;
    } catch {
      return false;
    }
  }

  private getConfigFileName(configProfile: string): string {
    if (configProfile === 'default') {
      return '.cortexrc';
    }
    return `.${configProfile}rc`;
  }

  public setConfigPath(configPath: string) {
    this.configPath = configPath;
  }

  public getConfigPath(): string {
    return this.configPath;
  }

  public setLogPath(logPath: string) {
    this.customLogPath = logPath;
  }
}

export const fileManagerService = new FileManagerService();
