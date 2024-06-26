import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { EnginesListCommand } from './engines/engines-list.command';
import { EnginesGetCommand } from './engines/engines-get.command';

@SubCommand({
  name: 'engines',
  subCommands: [EnginesListCommand, EnginesGetCommand],
  description: 'Get cortex engines',
})
@SetCommandContext()
export class EnginesCommand extends CommandRunner {
  constructor(readonly contextService: ContextService) {
    super();
  }

  async run(): Promise<void> {}
}
