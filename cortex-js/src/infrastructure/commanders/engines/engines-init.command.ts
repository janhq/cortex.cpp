import { CommandRunner, Option, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { InitCliUsecases } from '../usecases/init.cli.usecases';
import { Engines } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

@SubCommand({
  name: '<name> init',
  description: 'Setup engine',
  argsDescription: {
    name: 'Engine name to setup',
  },
})
@SetCommandContext()
export class EnginesInitCommand extends CommandRunner {
  constructor(
    private readonly initUsecases: InitCliUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly fileManagerService: FileManagerService,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(
    passedParams: string[],
    options: { vulkan: boolean },
  ): Promise<void> {
    const engine = passedParams[0];
    const params = passedParams.includes(Engines.llamaCPP)
      ? {
          ...(await this.initUsecases.defaultInstallationOptions()),
          ...options,
        }
      : {};

    const configs = await this.fileManagerService.getConfig();
    const host = configs.cortexCppHost;
    const port = configs.cortexCppPort;
    // Should stop cortex before installing engine
    if (await this.cortexUsecases.healthCheck(host, port)) {
      await this.cortexUsecases.stopCortex();
    }
    return this.initUsecases
      .installEngine(
        params,
        engine.includes('@') ? engine.split('@')[1] : 'latest',
        engine,
        true,
      )
      .then(() => console.log('Engine installed successfully!'))
      .catch((e) =>
        console.error('Install engine failed with reason: %s', e.message ?? e),
      );
  }

  @Option({
    flags: '-vk, --vulkan',
    description: 'Install Vulkan engine',
    defaultValue: false,
  })
  parseVulkan() {
    return true;
  }
}
