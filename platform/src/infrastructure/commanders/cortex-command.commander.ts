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
import {
  defaultCortexCppPort,
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '../constants/cortex';
import { getApp } from '@/app';
import { fileManagerService } from '../services/file-manager/file-manager.service';
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
  name?: string;
  configPath?: string;
  enginePort?: string;
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
  enginePort: number;
  constructor(
    readonly contextService: ContextService,
    readonly cortexUseCases: CortexUsecases,
  ) {
    super();
  }

  async run(passedParams: string[], options?: ServeOptions): Promise<void> {
    if (options?.configPath) {
      fileManagerService.setConfigPath(options.configPath);
    }
    if (options?.name) {
      fileManagerService.setConfigProfile(options.name);
    }
    if (options?.name) {
      const isProfileConfigExists = fileManagerService.profileConfigExists(
        options.name,
      );
      if (!isProfileConfigExists) {
        await fileManagerService.writeConfigFile({
          ...fileManagerService.defaultConfig(),
          apiServerHost: options?.address || defaultCortexJsHost,
          apiServerPort: options?.port || defaultCortexJsPort,
          cortexCppPort: Number(options?.enginePort) || defaultCortexCppPort,
        });
      }
    }
    const {
      apiServerHost: configApiServerHost,
      apiServerPort: configApiServerPort,
      cortexCppPort: configCortexCppPort,
    } = await fileManagerService.getConfig();

    this.configHost = configApiServerHost || defaultCortexJsHost;
    this.configPort = configApiServerPort || defaultCortexJsPort;

    this.host = options?.address || configApiServerHost || defaultCortexJsHost;
    this.port = options?.port || configApiServerPort || defaultCortexJsPort;
    if (this.host === 'localhost') {
      this.host = '127.0.0.1';
    }
    this.enginePort =
      Number(options?.enginePort) ||
      configCortexCppPort ||
      defaultCortexCppPort;
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
    const config = await fileManagerService.getConfig();
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
          `Server is already running at http://${this.configHost}:${this.configPort}. Please use 'cortex stop' to stop the server.`,
        );
        process.exit(0);
      }
      if (dataFolderPath) {
        await fileManagerService.writeConfigFile({
          ...config,
          dataFolderPath,
        });
        // load config again to create the data folder
        await fileManagerService.getConfig(dataFolderPath);
      }
      if (attach) {
        const app = await getApp(this.host, this.port);
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
      await fileManagerService.writeConfigFile({
        ...config,
        apiServerHost: this.host,
        apiServerPort: this.port,
        dataFolderPath: dataFolderPath || config.dataFolderPath,
        cortexCppPort: this.enginePort,
      });
      if (!attach) process.exit(0);
    } catch (e) {
      console.error(e);
      // revert the data folder path if it was set
      await fileManagerService.writeConfigFile({
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
    flags: '-df, --dataFolder <dataFolder>',
    description: 'Set the data folder directory',
  })
  parseDataFolder(value: string) {
    return value;
  }

  @Option({
    flags: '-cp, --configPath <configPath>',
    description: 'Set the config folder directory',
  })
  parseConfigFolder(value: string) {
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
    return value;
  }

  @Option({
    flags: '-ep, --engine-port <port>',
    description: 'Port to serve the engine',
  })
  parseEnginePort(value: string) {
    return value;
  }
}
