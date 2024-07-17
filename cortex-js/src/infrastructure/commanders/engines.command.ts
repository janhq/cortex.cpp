import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesListCommand } from './engines/engines-list.command';
import { EnginesGetCommand } from './engines/engines-get.command';
import { EnginesInitCommand } from './engines/engines-init.command';
import { ModuleRef } from '@nestjs/core';
import { EngineNamesMap } from './types/engine.interface';
import _ from 'lodash';

@SubCommand({
  name: 'engines',
  subCommands: [EnginesListCommand, EnginesGetCommand, EnginesInitCommand],
  description: 'Get cortex engines',
  arguments: '<command|parameter> [subcommand]',
})
@SetCommandContext()
export class EnginesCommand extends CommandRunner {
  commandMap: { [key: string]: any } = {
    list: EnginesListCommand,
    get: EnginesGetCommand,
    init: EnginesInitCommand,
  };

  constructor(
    readonly contextService: ContextService,
    private readonly moduleRef: ModuleRef,
  ) {
    super();
  }
  async run(passedParam: string[]): Promise<void> {
    const [parameter, command] = passedParam;
    if (command !== 'list' && !parameter) {
      console.error('Engine name is required.');
      return;
    }

    // Handle the commands accordingly
    const commandClass = this.commandMap[command as string];
    if (!commandClass) {
      this.command?.help();
      return;
    }
    const engine = _.invert(EngineNamesMap)[parameter] || parameter;
    await this.runCommand(commandClass, [engine]);
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
