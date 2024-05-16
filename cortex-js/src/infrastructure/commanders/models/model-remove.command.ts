import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { exit } from 'node:process';

@SubCommand({ name: 'remove' })
export class ModelRemoveCommand extends CommandRunner {
  constructor(private readonly modelsUsecases: ModelsUsecases) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    const modelsCliUsecases = new ModelsCliUsecases(this.modelsUsecases);
    const result = await modelsCliUsecases.removeModel(input[0]);
    console.log(result);
  }
}
