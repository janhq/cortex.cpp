import { Injectable } from '@nestjs/common';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';

@Injectable()
export class EnginesUsecases {
  constructor(private readonly extensionRepository: ExtensionRepository) {}

  /**
   * Get the engines
   * @returns Cortex supported Engines
   */
  async getEngines() {
    return (await this.extensionRepository.findAll()).map((e) => ({
      name: e.name,
      description: e.description,
      version: e.version,
      productName: e.productName,
    }));
  }

  /**
   * Get the engine with the given name
   * @param name Engine name
   * @returns Engine
   */
  async getEngine(name: string) {
    return this.extensionRepository.findOne(name).then((engine) =>
      engine
        ? {
            name: engine.name,
            description: engine.description,
            version: engine.version,
            productName: engine.productName,
          }
        : undefined,
    );
  }
}
