import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EngineNamesMap } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

@SubCommand({
  name: 'list',
  description: 'Get all cortex engines',
})
@SetCommandContext()
export class EnginesListCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
    readonly fileService: FileManagerService,
  ) {
    super(cortexUseCases, fileService);
  }

  async runCommand(): Promise<void> {
    return this.cortex.engines.list().then(({ data: engines }) => {
      const enginesTable = engines.map((engine) => ({
        ...engine,
        name: EngineNamesMap[engine.name as string] || engine.name,
      }));
      console.table(enginesTable);
    });
  }
}
