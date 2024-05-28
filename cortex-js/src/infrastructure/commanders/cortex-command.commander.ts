import { RootCommand, CommandRunner } from 'nest-commander';
import { ServeCommand } from './serve.command';
import { ChatCommand } from './chat.command';
import { ModelsCommand } from './models.command';
import { InitCommand } from './init.command';
import { RunCommand } from './shortcuts/run.command';
import { ModelPullCommand } from './models/model-pull.command';
import { PSCommand } from './ps.command';

@RootCommand({
  subCommands: [
    ModelsCommand,
    ServeCommand,
    ChatCommand,
    InitCommand,
    RunCommand,
    ModelPullCommand,
    PSCommand,
  ],
  description: 'Cortex CLI',
})
export class CortexCommand extends CommandRunner {
  async run(): Promise<void> {}
}
