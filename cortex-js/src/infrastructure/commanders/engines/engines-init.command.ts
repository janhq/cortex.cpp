import { CommandRunner, Option, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { Engines } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { BaseCommand } from '../base.command';

@SubCommand({
  name: '<name> init',
  description: 'Setup engine',
  argsDescription: {
    name: 'Engine name to setup',
  },
})
@SetCommandContext()
export class EnginesInitCommand extends BaseCommand {
  constructor(
    private readonly engineUsecases: EnginesUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly fileManagerService: FileManagerService,
    readonly contextService: ContextService,
  ) {
    super(cortexUsecases);
  }

  async runCommand(
    passedParams: string[],
    options: { vulkan: boolean },
  ): Promise<void> {
    const engine = passedParams[0];
    const params = passedParams.includes(Engines.llamaCPP)
      ? {
          ...(await this.engineUsecases.defaultInstallationOptions()),
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
    console.log(`Installing engine ${engine}...`);
    return this.engineUsecases
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
