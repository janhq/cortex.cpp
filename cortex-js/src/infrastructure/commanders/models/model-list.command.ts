import { SubCommand, Option } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseSubCommand } from '../base.subcommand';

interface ModelListOptions {
  format: 'table' | 'json';
}
@SubCommand({ name: 'list', description: 'List all models locally.' })
@SetCommandContext()
export class ModelListCommand extends BaseSubCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super(cortexUseCases);
  }

  async runCommand(
    passedParams: string[],
    option: ModelListOptions,
  ): Promise<void> {
    const { data: models } = await this.cortex.models.list();
    option.format === 'table'
      ? console.table(
          models.map((e) => ({
            id: e.id,
            engine: e.engine,
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
