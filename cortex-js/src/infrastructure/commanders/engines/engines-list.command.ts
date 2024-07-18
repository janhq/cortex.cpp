import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { EngineNamesMap } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';

@SubCommand({
  name: 'list',
  description: 'Get all cortex engines',
})
@SetCommandContext()
export class EnginesListCommand extends BaseCommand {
  constructor(
    private readonly enginesUsecases: EnginesUsecases,
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super(cortexUseCases);
  }

  async runCommand(): Promise<void> {
    return this.enginesUsecases.getEngines().then((engines) => {
      const enginesTable = engines.map((engine) => ({
        ...engine,
        name: EngineNamesMap[engine.name as string] || engine.name,
      }));
      console.table(enginesTable);
    });
  }
}
