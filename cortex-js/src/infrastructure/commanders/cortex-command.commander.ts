console.time('import-cortex-command-time');
console.time('import-cortex-command-commander');
import { RootCommand, CommandRunner } from 'nest-commander';
console.timeEnd('import-cortex-command-commander');
console.time('import-cortex-command-serve-command');
import { ServeCommand } from './serve.command';
console.timeEnd('import-cortex-command-serve-command');
console.time('import-cortex-command-chat-command');
import { ChatCommand } from './chat.command';
console.timeEnd('import-cortex-command-chat-command');
console.time('import-cortex-command-models-command');
import { ModelsCommand } from './models.command';
console.timeEnd('import-cortex-command-models-command');
console.time('import-cortex-command-init-command');
import { InitCommand } from './init.command';
console.timeEnd('import-cortex-command-init-command');
console.time('import-cortex-command-run-command');
import { RunCommand } from './shortcuts/run.command';
console.timeEnd('import-cortex-command-run-command');
console.time('import-cortex-command-model-pull-command');
import { ModelPullCommand } from './models/model-pull.command';
console.timeEnd('import-cortex-command-model-pull-command');
console.time('import-cortex-command-ps-command');
import { PSCommand } from './ps.command';
console.timeEnd('import-cortex-command-ps-command');
console.time('import-cortex-command-kill-command');
import { KillCommand } from './kill.command';
console.timeEnd('import-cortex-command-kill-command');
console.time('import-cortex-command-preset-command');
import pkg from '@/../package.json';
console.timeEnd('import-cortex-command-preset-command');
console.time('import-cortex-command-telemetry-command');
import { PresetCommand } from './presets.command';
console.timeEnd('import-cortex-command-telemetry-command');
console.time('import-cortex-command-telemetry-command');
import { TelemetryCommand } from './telemetry.command';
console.timeEnd('import-cortex-command-telemetry-command');
console.time('import-cortex-command-embedding-command');
import { SetCommandContext } from './decorators/CommandContext';
console.timeEnd('import-cortex-command-embedding-command');
console.time('import-cortex-command-benchmark-command');
import { EmbeddingCommand } from './embeddings.command';
console.timeEnd('import-cortex-command-benchmark-command');
console.time('import-cortex-command-engines-command');
import { BenchmarkCommand } from './benchmark.command';
console.timeEnd('import-cortex-command-engines-command');
console.time('import-cortex-command-configs-command');
import chalk from 'chalk';
console.timeEnd('import-cortex-command-configs-command');
console.time('import-cortex-command-configs-command');
import { printSlogan } from '@/utils/logo';
console.timeEnd('import-cortex-command-configs-command');
console.time('import-cortex-command-configs-command');
import { ContextService } from '../services/context/context.service';
console.timeEnd('import-cortex-command-configs-command');
console.time('import-cortex-command-configs-command');
import { EnginesCommand } from './engines.command';
console.timeEnd('import-cortex-command-configs-command');
console.time('import-cortex-command-configs-command');
import { ConfigsCommand } from './configs.command';
console.timeEnd('import-cortex-command-configs-command');
console.time('import-cortex-command-time');

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
    TelemetryCommand,
    EmbeddingCommand,
    BenchmarkCommand,
    EnginesCommand,
    ConfigsCommand,
  ],
  description: 'Cortex CLI',
})
@SetCommandContext()
export class CortexCommand extends CommandRunner {
  constructor(readonly contextService: ContextService) {
    super();
  }
  async run(): Promise<void> {
    printSlogan();
    console.log('\n');
    console.log(`Cortex CLI - v${pkg.version}`);
    console.log(chalk.blue(`Github: ${pkg.homepage}`));
    console.log('\n');
    this.command?.help();
  }
}
