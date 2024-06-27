import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';

@SubCommand({
  name: 'get',
  description: 'Get a cortex configuration',
  arguments: '<name>',
  argsDescription: {
    name: 'Configuration name to get',
  },
})
@SetCommandContext()
export class ConfigsGetCommand extends CommandRunner {
  constructor(
    private readonly configsUsecases: ConfigsUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(passedParams: string[]): Promise<void> {
    return this.configsUsecases
      .getGroupConfigs(passedParams[0])
      .then(console.table);
  }
}
