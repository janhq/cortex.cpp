import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { EngineNamesMap, Engines } from '../types/engine.interface';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: '<name> get',
  description: 'Get an engine',
  argsDescription: {
    name: 'Engine name to get',
  },
})
@SetCommandContext()
export class EnginesGetCommand extends BaseCommand {
  constructor(
    private readonly engineUsecases: EnginesUsecases,
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    return this.engineUsecases.getEngine(passedParams[0]).then((engine) => {
      if (!engine) {
        console.error('Engine not found.');
      } else {
        console.table({
          ...engine,
          name: EngineNamesMap[engine.name as Engines] || engine.name,
        });
      }
    });
  }
}
