import {
  CommandRunner,
  InquirerService,
  SubCommand,
  Option,
} from 'nest-commander';
import { InitCliUsecases } from './usecases/init.cli.usecases';
import { InitOptions } from './types/init-options.interface';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/util/context.service';

@SubCommand({
  name: 'init',
  aliases: ['setup'],
  description: "Init settings and download cortex's dependencies",
})
@SetCommandContext()
export class InitCommand extends CommandRunner {
  constructor(
    private readonly inquirerService: InquirerService,
    private readonly initUsecases: InitCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(input: string[], options?: InitOptions): Promise<void> {
    if (options?.silent) {
      return this.initSilently(input);
    } else {
      return this.initPrompts(input, options);
    }
  }

  private initSilently = async (input: string[], options: InitOptions = {}) => {
    const version = input[0] ?? 'latest';
    if (process.platform === 'darwin') {
      const engineFileName = this.initUsecases.parseEngineFileName(options);
      return this.initUsecases.installEngine(engineFileName, version);
    }
    // If Nvidia Driver is installed -> GPU
    options.runMode = (await this.initUsecases.checkNvidiaGPUExist())
      ? 'GPU'
      : 'CPU';
    // CPU Instructions detection
    options.gpuType = 'Nvidia';
    options.instructions = await this.initUsecases.detectInstructions();
    const engineFileName = this.initUsecases.parseEngineFileName(options);
    return this.initUsecases
      .installEngine(engineFileName, version)
      .then(() => this.initUsecases.installCudaToolkitDependency(options));
  };

  /**
   * Manual initalization
   * To setup cortex's dependencies
   * @param input
   * @param options GPU | CPU / Nvidia | Others (Vulkan) / AVX | AVX2 | AVX512
   */
  private initPrompts = async (input: string[], options?: InitOptions) => {
    options = await this.inquirerService.ask(
      'init-run-mode-questions',
      options,
    );

    if (options.runMode === 'GPU' && !(await this.initUsecases.cudaVersion())) {
      options = await this.inquirerService.ask('init-cuda-questions', options);
    }

    const version = input[0] ?? 'latest';

    const engineFileName = this.initUsecases.parseEngineFileName(options);
    await this.initUsecases.installEngine(engineFileName, version);

    if (options.installCuda === 'Yes') {
      await this.initUsecases.installCudaToolkitDependency(options);
    }
  };

  @Option({
    flags: '-s, --silent',
    description: 'Init without asking questions',
    defaultValue: false,
  })
  parseSilent() {
    return true;
  }
}
