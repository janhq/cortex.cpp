import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EngineNamesMap, Engines } from '../types/engine.interface';
import { BaseCommand } from '../base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseSubCommand } from '../base.subcommand';

@SubCommand({
  name: '<name> get',
  description: 'Get an engine',
  argsDescription: {
    name: 'Engine name to get',
  },
})
@SetCommandContext()
export class EnginesGetCommand extends BaseSubCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    return this.cortex.engines.retrieve(passedParams[0]).then((engine) => {
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
