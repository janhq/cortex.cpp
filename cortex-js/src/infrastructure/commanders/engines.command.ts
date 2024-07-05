import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesListCommand } from './engines/engines-list.command';
import { EnginesGetCommand } from './engines/engines-get.command';
import { EnginesInitCommand } from './engines/engines-init.command';

@SubCommand({
  name: 'engines',
  subCommands: [EnginesListCommand, EnginesGetCommand, EnginesInitCommand],
  description: 'Get cortex engines',
})
@SetCommandContext()
export class EnginesCommand extends CommandRunner {
  constructor(readonly contextService: ContextService) {
    super();
  }

  async run(): Promise<void> {
    this.command?.help();
  }
}
