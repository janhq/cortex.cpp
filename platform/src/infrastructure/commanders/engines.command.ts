import { invert } from 'lodash';
import { Option, SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { EnginesListCommand } from './engines/engines-list.command';
import { EnginesGetCommand } from './engines/engines-get.command';
import { EnginesInitCommand } from './engines/engines-init.command';
import { ModuleRef } from '@nestjs/core';
import { EngineNamesMap } from './types/engine.interface';
import { BaseCommand } from './base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { EnginesSetCommand } from './engines/engines-set.command';

@SubCommand({
  name: 'engines',
  subCommands: [EnginesListCommand, EnginesGetCommand, EnginesInitCommand],
  description: 'Get cortex engines',
  arguments: '<command|parameter> [subcommand]',
})
@SetCommandContext()
export class EnginesCommand extends BaseCommand {
  commandMap: { [key: string]: any } = {
    list: EnginesListCommand,
    get: EnginesGetCommand,
    init: EnginesInitCommand,
    set: EnginesSetCommand,
  };

  constructor(
    readonly contextService: ContextService,
    readonly moduleRef: ModuleRef,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }
  async runCommand(passedParams: string[], options: { vulkan: boolean }) {
    const [parameter, command] = passedParams;
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
    const engine = invert(EngineNamesMap)[parameter] || parameter;
    await this.runEngineCommand(
      commandClass,
      [engine, ...passedParams.slice(2)],
      options,
    );
  }

  private async runEngineCommand(
    commandClass: any,
    params: string[] = [],
    options?: { vulkan: boolean },
  ) {
    const commandInstance = this.moduleRef.get(commandClass, { strict: false });
    if (commandInstance) {
      commandInstance.setCortex(this.cortex);
      await commandInstance.runCommand(params, options);
    } else {
      console.error('Command not found.');
    }
  }

  @Option({
    flags: '-vk, --vulkan',
    description: 'Install Vulkan engine',
    defaultValue: false,
  })
  parseVulkan() {
    return true;
  }

  @Option({
    flags: '-v, --version <version>',
    description: 'Select version to install',
  })
  parseVersion(value: string) {
    return value;
  }
}
