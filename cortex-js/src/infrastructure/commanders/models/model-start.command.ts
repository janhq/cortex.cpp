import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';

@SubCommand({ name: 'start' })
export class ModelStartCommand extends CommandRunner {
  constructor(private readonly modelsUsecases: ModelsUsecases) {
    super();
  }

  async run(input: string[]): Promise<void> {
    if (input.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    const modelsCliUsecases = new ModelsCliUsecases(this.modelsUsecases);
    await modelsCliUsecases.startModel(input[0]);
  }
}
