import { SubCommand, Option } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { CortexClient } from '../services/cortex.client';

interface ModelListOptions {
  format: 'table' | 'json';
}
@SubCommand({ name: 'list', description: 'List all models locally.' })
@SetCommandContext()
export class ModelListCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
    private readonly cortex: CortexClient,
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
