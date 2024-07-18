import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';

@SubCommand({
  name: 'get',
  description: 'Get a cortex configuration',
  arguments: '<name>',
  argsDescription: {
    name: 'Configuration name to get',
  },
})
@SetCommandContext()
export class ConfigsGetCommand extends BaseCommand {
  constructor(
    private readonly configsUsecases: ConfigsUsecases,
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(passedParams: string[]): Promise<void> {
    return this.configsUsecases
      .getGroupConfigs(passedParams[0])
      .then(console.table);
  }
}
