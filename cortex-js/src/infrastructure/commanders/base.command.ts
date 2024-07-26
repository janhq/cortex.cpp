import { CommandRunner } from 'nest-commander';
import { Injectable } from '@nestjs/common';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import Cortex from '@cortexso/cortex.js';
import ora from 'ora';
import { FileManagerService } from '../services/file-manager/file-manager.service';
import { cortexNamespace, cortexServerAPI } from '../constants/cortex';

@Injectable()
export abstract class BaseCommand extends CommandRunner {
  // Cortex client instance to communicate with cortex API server
  cortex: Cortex;
  serverConfigs: { host: string; port: number };

  constructor(readonly cortexUseCases: CortexUsecases) {
    super();
    // No need to inject services, since there is no nested dependencies
    const fileManagerService: FileManagerService = new FileManagerService();
    this.serverConfigs = fileManagerService.getServerConfig();

    // Instantiate cortex client, it will be use throughout the command
    this.cortex = new Cortex({
      apiKey: cortexNamespace,
      baseURL: cortexServerAPI(
        this.serverConfigs.host,
        this.serverConfigs.port,
      ),
      timeout: 20 * 1000,
    });
  }
  protected abstract runCommand(
    passedParam: string[],
    options?: Record<string, any>,
  ): Promise<void>;

  async run(
    passedParam: string[],
    options?: Record<string, any>,
  ): Promise<void> {
    const checkingSpinner = ora('Checking API server online...').start();
    const result = await this.cortexUseCases.isAPIServerOnline();
    if (!result) {
      checkingSpinner.fail('API server is offline');
      process.exit(1);
    }
    checkingSpinner.succeed('API server is online');
    await this.runCommand(passedParam, options);
  }
}
