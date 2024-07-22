import { SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { ConfigsGetCommand } from './configs/configs-get.command';
import { ConfigsListCommand } from './configs/configs-list.command';
import { ConfigsSetCommand } from './configs/configs-set.command';
import { BaseCommand } from './base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

@SubCommand({
  name: 'configs',
  description: 'Get cortex configurations',
  subCommands: [ConfigsGetCommand, ConfigsListCommand, ConfigsSetCommand],
})
@SetCommandContext()
export class ConfigsCommand extends BaseCommand {
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super(cortexUseCases);
  }

  async runCommand(): Promise<void> {
    this.command?.help();
  }
}
