import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/util/context.service';

interface ModelListOptions {
  format: 'table' | 'json';
}
@SubCommand({ name: 'list', description: 'List all models locally.' })
@SetCommandContext()
export class ModelListCommand extends CommandRunner {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(passedParams: string[], option: ModelListOptions): Promise<void> {
    const models = await this.modelsCliUsecases.listAllModels();
    option.format === 'table'
      ? console.table(
          models.map((e) => ({
            id: e.model,
            engine: e.engine,
            version: e.version,
          })),
        )
      : console.log(models);
  }

  @Option({
    flags: '-f, --format <format>',
    defaultValue: 'table',
    description: 'Print models list in table or json format',
  })
  parseModelId(value: string) {
    return value;
  }
}
