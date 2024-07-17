import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesListCommand } from './engines/engines-list.command';
import { EnginesGetCommand } from './engines/engines-get.command';
import { EnginesInitCommand } from './engines/engines-init.command';
import { ModuleRef } from '@nestjs/core';

@SubCommand({
  name: 'engines',
  subCommands: [EnginesListCommand, EnginesGetCommand, EnginesInitCommand],
  description: 'Get cortex engines',
  arguments: '<command|parameter> [subcommand]',
})
@SetCommandContext()
export class EnginesCommand extends CommandRunner {
  constructor(
    readonly contextService: ContextService,
    private readonly moduleRef: ModuleRef,
  ) {
    super();
  }

  async run(passedParam: string[]): Promise<void> {
    console.log('Engines command');
    const [parameter, command] = passedParam;
    if (command === 'init' && !parameter) {
      console.error('Parameter is required for the init command.');
      return;
    }

    // Handle the commands accordingly
    if (command === 'init') {
      await this.runCommand(EnginesInitCommand, [parameter]);
    } else {
      this.command?.help();
    }
  }

  private async runCommand(commandClass: any, params: string[] = []) {
    const commandInstance = this.moduleRef.get(commandClass, { strict: false });
    if (commandInstance) {
      await commandInstance.run(params);
    } else {
      console.error('Command not found.');
    }
  }
}
