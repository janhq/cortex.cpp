import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { Engines } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';
import { defaultInstallationOptions } from '@/utils/init';
import { Presets, SingleBar } from 'cli-progress';
import ora from 'ora';
import { InitEngineDto } from '@/infrastructure/dtos/engines/engines.dto';
import { fileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

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
    readonly contextService: ContextService,
  ) {
    super(cortexUsecases);
  }

  async runCommand(
    passedParams: string[],
    options: InitEngineDto,
  ): Promise<void> {
    const engine = passedParams[0];
    const params = passedParams.includes(Engines.llamaCPP)
      ? {
          ...(await defaultInstallationOptions()),
          ...options,
        }
      : {};

    const configs = await fileManagerService.getConfig();
    const host = configs.cortexCppHost;
    const port = configs.cortexCppPort;
    // Should stop cortex before installing engine
    const stopCortexSpinner = ora('Stopping cortex...').start();
    if (await this.cortexUsecases.healthCheck(host, port)) {
      await this.cortexUsecases.stopCortex();
    }
    stopCortexSpinner.succeed('Cortex stopped');
    console.log(`Installing engine ${engine}...`);

    await this.cortex.engines.init(engine, params);
    const response = await this.cortex.events.downloadEvent();

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
    const startCortexSpinner = ora('Starting cortex...').start();
    await this.cortexUsecases.startCortex();
    startCortexSpinner.succeed('Cortex started');
    console.log('Engine installed successfully');
    process.exit(0);
  }
}
