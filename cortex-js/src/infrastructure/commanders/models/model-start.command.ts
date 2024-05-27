import { CommandRunner, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({ name: 'start', description: 'Start a model by ID.' })
export class ModelStartCommand extends CommandRunner {
  constructor(
    private readonly cortexUsecases: CortexUsecases,
    private readonly modelsCliUsecases: ModelsCliUsecases,
  ) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    await this.cortexUsecases.startCortex();
    await this.modelsCliUsecases.startModel(input[0]);
  }
}
