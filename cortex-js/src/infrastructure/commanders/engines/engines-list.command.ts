import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { EngineNamesMap } from '../types/engine.interface';

@SubCommand({
  name: 'list',
  description: 'Get all cortex engines',
})
@SetCommandContext()
export class EnginesListCommand extends CommandRunner {
  constructor(
    private readonly enginesUsecases: EnginesUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(): Promise<void> {
    return this.enginesUsecases.getEngines().then((engines) => {
      const enginesTable = engines.map((engine) => ({
        ...engine,
        name: EngineNamesMap[engine.name as string] || engine.name,
      }));
      console.table(enginesTable);
    });
  }
}
