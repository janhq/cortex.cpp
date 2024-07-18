import { RootCommand, CommandRunner, Option } from 'nest-commander';
import { ChatCommand } from './chat.command';
import { ModelsCommand } from './models.command';
import { RunCommand } from './shortcuts/run.command';
import { ModelPullCommand } from './models/model-pull.command';
import { PSCommand } from './ps.command';
import { KillCommand } from './kill.command';
import { PresetCommand } from './presets.command';
import { TelemetryCommand } from './telemetry.command';
import { SetCommandContext } from './decorators/CommandContext';
import { EmbeddingCommand } from './embeddings.command';
import { BenchmarkCommand } from './benchmark.command';
import chalk from 'chalk';
import { ContextService } from '../services/context/context.service';
import { EnginesCommand } from './engines.command';
import { ConfigsCommand } from './configs.command';
import { defaultCortexJsHost, defaultCortexJsPort } from '../constants/cortex';
import { getApp } from '@/app';
import { FileManagerService } from '../services/file-manager/file-manager.service';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

type ServeOptions = {
  address?: string;
  port?: number;
  attach?: boolean;
};

@RootCommand({
  subCommands: [
    ModelsCommand,
    ChatCommand,
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
  constructor(
    readonly contextService: ContextService,
    readonly fileManagerService: FileManagerService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super();
  }

  async run(passedParams: string[], options?: ServeOptions): Promise<void> {
    const host = options?.address || defaultCortexJsHost;
    const port = options?.port || defaultCortexJsPort;
    const attach = options?.attach || false;

    return this.startServer(host, port, attach);
  }

  private async startServer(host: string, port: number, attach: boolean) {
    console.log(attach);
    try {
      if (attach) {
        const app = await getApp();
        await app.listen(port, host);
      } else {
        await this.cortexUseCases.startServerDetached(host, port);
        console.log(
          chalk.blue(
            `Started server at http://${host}:${port} in detached mode`,
          ),
        );
      }
      console.log(chalk.blue(`Started server at http://${host}:${port}`));
      console.log(
        chalk.blue(`API Playground available at http://${host}:${port}/api`),
      );
      const config = await this.fileManagerService.getConfig();
      await this.fileManagerService.writeConfigFile({
        ...config,
        apiServerHost: host,
        apiServerPort: port,
      });
    } catch (e) {
      console.error(e);
      console.error(`Failed to start server. Is port ${port} in use?`);
    }
  }

  @Option({
    flags: '-a, --address <address>',
    description: 'Address to use',
  })
  parseHost(value: string) {
    return value;
  }

  @Option({
    flags: '-p, --port <port>',
    description: 'Port to serve the application',
  })
  parsePort(value: string) {
    return parseInt(value, 10);
  }

  @Option({
    flags: '-A, --attach',
    description: 'Attach to the process',
  })
  parseAttach() {
    return true;
  }
}
