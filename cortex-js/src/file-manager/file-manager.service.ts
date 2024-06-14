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

const readFileAsync = promisify(read);
const openAsync = promisify(open);
const closeAsync = promisify(close);
const writeAsync = promisify(write);
@Injectable()
export class FileManagerService {
  private configFile = '.cortexrc';
  private cortexDirectoryName = 'cortex';
  private modelFolderName = 'models';
  private cortexCppFolderName = 'cortex-cpp';
  private cortexTelemetryFolderName = 'telemetry';

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
      return config;
    } catch (error) {
      console.warn('Error reading config file. Using default config.');
      console.warn(error);
      const config = this.defaultConfig();
      await this.createFolderIfNotExist(config.dataFolderPath);
      await this.writeConfigFile(config);
      return config;
    }
  }

  private async writeConfigFile(config: Config): Promise<void> {
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
    };
  }

  async getDataFolderPath(): Promise<string> {
    const config = await this.getConfig();
    return config.dataFolderPath;
  }

  async getLastLine(
    filePath: string,
  ): Promise<{ data: any; position: number }> {
    try {
      const fileDescriptor = await openAsync(filePath, 'r');
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
      console.error('Error reading last line:', err);
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
      console.error('Error modifying last line:', err);
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
        },
      );
    } catch (err) {
      console.error('Error appending to file:', err);
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
}
