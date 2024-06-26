import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';

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
    return this.enginesUsecases.getEngines().then(console.table);
  }
}
