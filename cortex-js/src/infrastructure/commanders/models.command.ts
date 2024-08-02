import { SubCommand } from 'nest-commander';
import { ModelStartCommand } from './models/model-start.command';
import { ModelGetCommand } from './models/model-get.command';
import { ModelListCommand } from './models/model-list.command';
import { ModelStopCommand } from './models/model-stop.command';
import { ModelRemoveCommand } from './models/model-remove.command';
import { ModelUpdateCommand } from './models/model-update.command';
import { BaseCommand } from './base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { ModuleRef } from '@nestjs/core';
import { ModelPullCommand } from './models/model-pull.command';

@SubCommand({
  name: 'models',
  subCommands: [
    ModelStartCommand,
    ModelStopCommand,
    ModelListCommand,
    ModelGetCommand,
    ModelRemoveCommand,
    ModelUpdateCommand,
  ],
  description: 'Subcommands for managing models',
})
export class ModelsCommand extends BaseCommand {
  constructor(
    private readonly moduleRef: ModuleRef,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  commandMap: { [key: string]: any } = {
    pull: ModelPullCommand,
    start: ModelStartCommand,
    stop: ModelStopCommand,
    list: ModelListCommand,
    get: ModelGetCommand,
    remove: ModelRemoveCommand,
    update: ModelUpdateCommand,
  };
  async runCommand(passedParam: string[], options: any) {
    const [parameter, command, ...restParams] = passedParam;
    if (command !== 'list' && !parameter) {
      console.error('Model id is required.');
      return;
    }

    // Handle the commands accordingly
    const commandClass = this.commandMap[command as string];
    if (!commandClass) {
      this.command?.help();
      return;
    }
    const modelId = parameter;
    await this.runModelCommand(
      commandClass,
      [modelId, ...(restParams || [])],
      options,
    );
  }

  private async runModelCommand(
    commandClass: any,
    params: string[] = [],
    options?: any,
  ) {
    const commandInstance = this.moduleRef.get(commandClass, { strict: false });
    if (commandInstance) {
      await commandInstance.runCommand(params, options);
    } else {
      console.error('Command not found.');
    }
  }

  help() {
    console.log('Usage: cortex models <subcommand|parameter> [subcommand]');
    console.log('Commands:');
    console.log('  <model_id> start  - Start a model by ID');
    console.log('  <model_id> stop  - Stop a model by ID');
    console.log('  list - List all available models');
    console.log('  <model_id> get  - Get model details by ID');
    console.log('  <model_id> remove  - Remove a model by ID');
    console.log('  <model_id> update  - Update a model by ID');
  }
}
