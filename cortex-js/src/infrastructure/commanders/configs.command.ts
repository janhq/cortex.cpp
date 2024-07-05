import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { ConfigsGetCommand } from './configs/configs-get.command';
import { ConfigsListCommand } from './configs/configs-list.command';
import { ConfigsSetCommand } from './configs/configs-set.command';

@SubCommand({
  name: 'configs',
  description: 'Get cortex configurations',
  subCommands: [ConfigsGetCommand, ConfigsListCommand, ConfigsSetCommand],
})
@SetCommandContext()
export class ConfigsCommand extends CommandRunner {
  constructor(readonly contextService: ContextService) {
    super();
  }

  async run(): Promise<void> {
    this.command?.help();
  }
}
