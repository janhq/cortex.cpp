import { RootCommand, CommandRunner, Option } from 'nest-commander';
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

interface CortexCommandOptions {
  version: boolean;
}
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
  async run(input: string[], option: CortexCommandOptions): Promise<void> {
    if (option.version) console.log(pkg.version);
    else this.command?.help();
  }

  @Option({
    flags: '-v, --version',
    description: 'Cortex version',
    defaultValue: false,
    name: 'version',
  })
  parseVersion() {
    return true;
  }
}
