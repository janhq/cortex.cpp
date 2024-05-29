import { CommandRunner, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({ name: 'stop', description: 'Stop a model by ID.' })
export class ModelStopCommand extends CommandRunner {
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

    await this.modelsCliUsecases
      .stopModel(input[0])
      .then(() => this.cortexUsecases.stopCortex())
      .then(console.log);
  }
}
