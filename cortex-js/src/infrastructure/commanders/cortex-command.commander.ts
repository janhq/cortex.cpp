import { RootCommand, CommandRunner } from 'nest-commander';
import { ServeCommand } from './serve.command';
import { ChatCommand } from './chat.command';
import { ModelsCommand } from './models.command';
import { InitCommand } from './init.command';
import { RunCommand } from './shortcuts/run.command';
import { ModelPullCommand } from './models/model-pull.command';
import { PSCommand } from './ps.command';
import { KillCommand } from './kill.command';
import pkg from '@/../package.json';
import { PresetCommand } from './presets.command';
import { EmbeddingCommand } from './embeddings.command';
import { BenchmarkCommand } from './benchmark.command';
import chalk from 'chalk';
import { printSlogan } from '@/utils/logo';

@RootCommand({
  subCommands: [
    ModelsCommand,
    ServeCommand,
    ChatCommand,
    InitCommand,
    RunCommand,
    ModelPullCommand,
    PSCommand,
    KillCommand,
    PresetCommand,
    EmbeddingCommand,
    BenchmarkCommand,
  ],
  description: 'Cortex CLI',
})
export class CortexCommand extends CommandRunner {
  async run(): Promise<void> {
    printSlogan();
    console.log('\n');
    console.log(`Cortex CLI - v${pkg.version}`);
    console.log(chalk.blue(`Github: ${pkg.homepage}`));
    console.log('\n');
    this.command?.help();
  }
}
