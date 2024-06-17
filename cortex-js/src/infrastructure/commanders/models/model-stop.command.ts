import { CommandRunner, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '@commanders/usecases/models.cli.usecases';

@SubCommand({
  name: 'stop',
  description: 'Stop a model by ID.',
  arguments: '<model_id>',
  argsDescription: {
    model_id: 'Model ID to stop.',
  },
})
export class ModelStopCommand extends CommandRunner {
  constructor(private readonly modelsCliUsecases: ModelsCliUsecases) {
    super();
  }

  async run(passedParams: string[]): Promise<void> {
    if (passedParams.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    await this.modelsCliUsecases.stopModel(passedParams[0]).then(console.log);
  }
}
