import { Injectable } from '@nestjs/common';
import { ConfigsUsecases } from '../configs/configs.usecase';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';

@Injectable()
export class EnginesUsecases {
  constructor(
    private readonly configsUsechases: ConfigsUsecases,
    private readonly extensionRepository: ExtensionRepository,
  ) {}

  /**
   * Get the engines
   * @returns Cortex supported Engines
   */
  async getEngines() {
    return this.extensionRepository.findAll();
  }

  /**
   * Get the engine with the given name
   * @param name Engine name
   * @returns Engine
   */
  async getEngine(name: string) {
    return {
      engine: await this.extensionRepository.findOne(name),
      configs: await this.configsUsechases.getGroupConfigs(name),
    };
  }
}
