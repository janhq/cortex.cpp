import { Injectable } from '@nestjs/common';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import Cortex from '@cortexso/cortex.js';
import { FileManagerService } from '../services/file-manager/file-manager.service';
import { cortexNamespace, cortexServerAPI } from '../constants/cortex';
import { BaseCommand } from './base.command';

@Injectable()
export abstract class BaseSubCommand extends BaseCommand {
  // Cortex client instance to communicate with cortex API server
  cortex: Cortex;
  serverConfigs: { host: string; port: number };

  constructor(readonly cortexUseCases: CortexUsecases) {
    super(cortexUseCases);
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
}
