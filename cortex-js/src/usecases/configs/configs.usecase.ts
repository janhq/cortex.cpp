import { CommonResponseDto } from '@/infrastructure/dtos/common/common-response.dto';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { Injectable } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';

@Injectable()
export class ConfigsUsecases {
  constructor(
    private readonly fileManagerService: FileManagerService,
    private readonly eventEmitter: EventEmitter2,
  ) {}

  /**
   * Save a configuration to the .cortexrc file.
   * @param key Configuration Key
   * @param engine The engine where the configs belongs
   */
  async saveConfig(
    key: string,
    value: string,
    engine?: string,
  ): Promise<CommonResponseDto> {
    const configs = await this.fileManagerService.getConfig();

    const groupConfigs = configs[
      engine as keyof typeof configs
    ] as unknown as object;
    const newConfigs = {
      ...configs,
      ...(engine
        ? {
            [engine]: {
              ...groupConfigs,
              [key]: value,
            },
          }
        : {}),
    };

    return this.fileManagerService
      .writeConfigFile(newConfigs)
      .then(async () => {
        if (engine) {
          this.eventEmitter.emit('config.updated', {
            engine,
            key,
            value,
          });
        }
      })
      .then(() => {
        return {
          message: 'The config has been successfully updated.',
        };
      });
  }

  /**
   * Get the configurations of a group.
   * @param group
   * @returns
   */
  async getGroupConfigs(group: string) {
    const configs = await this.fileManagerService.getConfig();
    return configs[group as keyof typeof configs] as unknown as object;
  }

  /**
   * Get all available configurations.
   * @param group
   * @returns
   */
  async getConfigs() {
    return this.fileManagerService.getConfig();
  }

  /**
   * Get the configuration with given key
   * @param group
   * @param key
   * @returns
   */
  async getKeyConfig(key: string) {
    const configs = await this.fileManagerService.getConfig();
    return configs[key as keyof typeof configs];
  }
}
