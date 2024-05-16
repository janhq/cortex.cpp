import { CommandRunner, SubCommand } from 'nest-commander';
import { ModelStartCommand } from './models/model-start.command';
import { ModelGetCommand } from './models/model-get.command';
import { ModelListCommand } from './models/model-list.command';
import { ModelStopCommand } from './models/model-stop.command';
import { ModelPullCommand } from './models/model-pull.command';
import { ModelRemoveCommand } from './models/model-remove.command';

@SubCommand({
  name: 'models',
  subCommands: [
    ModelPullCommand,
    ModelStartCommand,
    ModelStopCommand,
    ModelListCommand,
    ModelGetCommand,
    ModelRemoveCommand,
  ],
})
export class ModelsCommand extends CommandRunner {
  async run(): Promise<void> {}
}
