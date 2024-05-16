import { RootCommand, CommandRunner } from 'nest-commander';
import { ServeCommand } from './serve.command';
import { ChatCommand } from './chat.command';
import { ModelsCommand } from './models.command';
import { InitCommand } from './init.command';
import { StartCommand } from './start.command';

@RootCommand({
  subCommands: [
    ModelsCommand,
    ServeCommand,
    ChatCommand,
    InitCommand,
    StartCommand,
  ],
})
export class CortexCommand extends CommandRunner {
  async run(): Promise<void> {}
}
