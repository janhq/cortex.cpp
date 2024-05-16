import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';

@SubCommand({ name: 'list', description: 'List all models locally.' })
export class ModelListCommand extends CommandRunner {
  constructor(private readonly modelsUsecases: ModelsUsecases) {
    super();
  }

  async run(): Promise<void> {
    const modelsCliUsecases = new ModelsCliUsecases(this.modelsUsecases);
    const models = await modelsCliUsecases.listAllModels();
    console.log(models);
  }
}
