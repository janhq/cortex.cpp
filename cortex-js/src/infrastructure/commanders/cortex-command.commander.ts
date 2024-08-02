import pkg from '@/../package.json';
import { RootCommand, CommandRunner, Option } from 'nest-commander';
import { ChatCommand } from './chat.command';
import { ModelsCommand } from './models.command';
import { RunCommand } from './run.command';
import { ModelPullCommand } from './models/model-pull.command';
import { PSCommand } from './ps.command';
import { PresetCommand } from './presets.command';
import { TelemetryCommand } from './telemetry.command';
import { SetCommandContext } from './decorators/CommandContext';
import { EmbeddingCommand } from './embeddings.command';
import { BenchmarkCommand } from './benchmark.command';
import chalk from 'chalk';
import { ContextService } from '../services/context/context.service';
import { EnginesCommand } from './engines.command';
import { defaultCortexJsHost, defaultCortexJsPort } from '../constants/cortex';
import { getApp } from '@/app';
import { FileManagerService } from '../services/file-manager/file-manager.service';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { ServeStopCommand } from './serve-stop.command';
import ora from 'ora';
import { EnginesSetCommand } from './engines/engines-set.command';

type ServeOptions = {
  address?: string;
  port?: number;
  logs?: boolean;
  dataFolder?: string;
  version?: boolean;
};

@RootCommand({
  subCommands: [
    ModelsCommand,
    ChatCommand,
    RunCommand,
    ModelPullCommand,
    PSCommand,
    PresetCommand,
    TelemetryCommand,
    EmbeddingCommand,
    BenchmarkCommand,
    EnginesCommand,
    ServeStopCommand,
    EnginesSetCommand,
  ],
  description: 'Cortex CLI',
})
@SetCommandContext()
export class CortexCommand extends CommandRunner {
  host: string;
  port: number;
  configHost: string;
  configPort: number;
  constructor(
    readonly contextService: ContextService,
    readonly fileManagerService: FileManagerService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super();
  }

  async run(passedParams: string[], options?: ServeOptions): Promise<void> {
    const {
      apiServerHost: configApiServerHost,
      apiServerPort: configApiServerPort,
    } = await this.fileManagerService.getConfig();

    this.configHost = configApiServerHost || defaultCortexJsHost;
    this.configPort = configApiServerPort || defaultCortexJsPort;

    this.host = options?.address || configApiServerHost || defaultCortexJsHost;
    this.port = options?.port || configApiServerPort || defaultCortexJsPort;

    const showLogs = options?.logs || false;
    const showVersion = options?.version || false;
    const dataFolderPath = options?.dataFolder;
    if (showVersion) {
      console.log('\n');
      console.log(`Cortex CLI - v${pkg.version}`);
      console.log(chalk.blue(`Github: ${pkg.homepage}`));
      return;
    }
    return this.startServer(showLogs, dataFolderPath);
  }

  private async startServer(attach: boolean, dataFolderPath?: string) {
    const config = await this.fileManagerService.getConfig();
    try {
      const startEngineSpinner = ora('Starting Cortex engine...');
      await this.cortexUseCases.startCortex().catch((e) => {
        startEngineSpinner.fail('Failed to start Cortex engine');
        throw e;
      });
      startEngineSpinner.succeed('Cortex started successfully');
      const isServerOnline = await this.cortexUseCases.isAPIServerOnline();
      if (isServerOnline) {
        console.log(
          chalk.blue(
            `Server is already running at http://${this.configHost}:${this.configPort}. Please use 'cortex stop' to stop the server.`,
          ),
        );
        process.exit(0);
      }
      if (dataFolderPath) {
        await this.fileManagerService.writeConfigFile({
          ...config,
          dataFolderPath,
        });
        // load config again to create the data folder
        await this.fileManagerService.getConfig(dataFolderPath);
      }
      if (attach) {
        const app = await getApp();
        await app.listen(this.port, this.host);
      } else {
        await this.cortexUseCases.startServerDetached(this.host, this.port);
      }
      console.log(
        chalk.blue(`Started server at http://${this.host}:${this.port}`),
      );
      console.log(
        chalk.blue(
          `API Playground available at http://${this.host}:${this.port}/api`,
        ),
      );
      await this.fileManagerService.writeConfigFile({
        ...config,
        apiServerHost: this.host,
        apiServerPort: this.port,
        dataFolderPath: dataFolderPath || config.dataFolderPath,
      });
      if (!attach) process.exit(0);
    } catch (e) {
      console.error(e);
      // revert the data folder path if it was set
      await this.fileManagerService.writeConfigFile({
        ...config,
      });
      console.error(`Failed to start server. Is port ${this.port} in use?`);
      process.exit(1);
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
    flags: '-l, --logs',
    description: 'Show logs',
  })
  parseLogs() {
    return true;
  }

  @Option({
    flags: '--dataFolder <dataFolderPath>',
    description: 'Set the data folder directory',
  })
  parseDataFolder(value: string) {
    return value;
  }

  @Option({
    flags: '-v, --version',
    description: 'Show version',
  })
  parseVersion() {
    return true;
  }
  @Option({
    flags: '-n, --name <name>',
    description: 'Name of the process',
  })
  parseName(value: string) {
    this.fileManagerService.setConfigProfile(value);
    return value;
  }
}
