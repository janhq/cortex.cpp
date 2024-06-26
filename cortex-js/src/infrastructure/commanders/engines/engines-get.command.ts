import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';

@SubCommand({
  name: 'get',
  description: 'Get an engine',
  arguments: '<name>',
  argsDescription: {
    name: 'Engine name to get',
  },
})
@SetCommandContext()
export class EnginesGetCommand extends CommandRunner {
  constructor(
    private readonly engineUsecases: EnginesUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(passedParams: string[]): Promise<void> {
    return this.engineUsecases.getEngine(passedParams[0]).then(console.table);
  }
}
