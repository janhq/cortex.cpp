import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';

@SubCommand({ name: 'list', description: 'List all models locally.' })
export class ModelListCommand extends CommandRunner {
  constructor(private readonly modelsCliUsecases: ModelsCliUsecases) {
    super();
  }

  async run(): Promise<void> {
    const models = await this.modelsCliUsecases.listAllModels();
    console.log(models);
  }
}
