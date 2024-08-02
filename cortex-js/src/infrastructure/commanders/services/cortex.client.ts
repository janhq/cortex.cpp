import {
  cortexNamespace,
  cortexServerAPI,
} from '@/infrastructure/constants/cortex';
import { fileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import Cortex from '@cortexso/cortex.js';

export class CortexClient extends Cortex {
  serverConfigs: { host: string; port: number };

  constructor() {
    const configs = fileManagerService.getServerConfig();
    super({
      baseURL: cortexServerAPI(configs.host, configs.port),
      apiKey: cortexNamespace,
    });
    this.serverConfigs = configs;
  }
}
