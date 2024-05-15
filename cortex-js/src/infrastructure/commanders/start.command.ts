import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { resolve } from 'path';
import { existsSync } from 'fs';
import { Model, ModelSettingParams } from '@/domain/models/model.interface';
import { exit } from 'node:process';

@SubCommand({ name: 'start', aliases: ['run'] })
export class StartCommand extends CommandRunner {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    private readonly cortexUsecases: CortexUsecases,
  ) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }
    const modelId = input[0];
    const model = await this.getModelOrStop(modelId);

    return this.startCortex()
      .then(() => this.startModel(model.id))
      .then(console.log)
      .catch(console.error);
  }

  private async startCortex() {
    if (!existsSync(resolve(this.rootDir(), 'cortex-cpp'))) {
      console.log('Please init the cortex by running cortex init command!');
      exit(0);
    }
    const host = '127.0.0.1';
    const port = 3928;
    return this.cortexUsecases.startCortex(host, port);
  }

  private async startModel(modelId: string) {
    return this.modelsUsecases.startModel(modelId);
  }

  private async getModelOrStop(modelId: string): Promise<Model> {
    const model = await this.modelsUsecases.findOne(modelId);
    if (!model) {
      console.debug('Model not found');
      exit(1);
    }
    return model;
  }

  rootDir = () => resolve(__dirname, `../../../`);
}
