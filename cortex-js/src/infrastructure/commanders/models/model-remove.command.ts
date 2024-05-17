import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { exit } from 'node:process';

@SubCommand({ name: 'remove', description: 'Remove a model by ID locally.' })
export class ModelRemoveCommand extends CommandRunner {
  constructor(private readonly modelsCliUsecases: ModelsCliUsecases) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    const result = await this.modelsCliUsecases.removeModel(input[0]);
    console.log(result);
  }
}
