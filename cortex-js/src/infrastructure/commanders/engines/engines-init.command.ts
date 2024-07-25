import { exit, stdin, stdout } from 'node:process';
import { Option, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { Engines } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { BaseCommand } from '../base.command';
import { defaultInstallationOptions } from '@/utils/init';
import { Presets, SingleBar } from 'cli-progress';

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
          ...(await defaultInstallationOptions()),
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
    await this.cortex.engines
      .init(engine, params)
      const response = await this.cortex.models.downloadEvent()
  
      const progressBar = new SingleBar({}, Presets.shades_classic);
      progressBar.start(100, 0);
  
      for await (const stream of response) {
        if (stream.length) {
          const data = stream[0] as any;
          if (data.status === 'downloaded') break;
          let totalBytes = 0;
          let totalTransferred = 0;
          data.children.forEach((child: any) => {
            totalBytes += child.size.total;
            totalTransferred += child.size.transferred;
          });
          progressBar.update(Math.floor((totalTransferred / totalBytes) * 100));
        }
      }
      progressBar.stop();
      console.log('Engine installed successfully');
      process.exit(0);
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
