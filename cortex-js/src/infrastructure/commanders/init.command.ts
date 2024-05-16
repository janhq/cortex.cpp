
import { CommandRunner, InquirerService, SubCommand } from 'nest-commander';
import { InitCliUsecases } from './usecases/init.cli.usecases';
import { InitOptions } from './types/init-options.interface';


@SubCommand({
  name: 'init',
  aliases: ['setup'],
  description: "Init settings and download cortex's dependencies",
})
export class InitCommand extends CommandRunner {

  constructor(
    private readonly inquirerService: InquirerService,
    private readonly initUsecases: InitCliUsecases,
  ) {
    super();
  }

  async run(input: string[], options?: InitOptions): Promise<void> {
    options = await this.inquirerService.ask('init-run-mode-questions', options);

    if (options.runMode === 'GPU' && !(await this.initUsecases.cudaVersion())) {
      options = await this.inquirerService.ask('init-cuda-questions', options);
    }

    const version = input[0] ?? 'latest';

    const engineFileName = this.initUsecases.parseEngineFileName(options)
    await this.initUsecases.installEngine(engineFileName, version);

    if (options.installCuda === 'Yes') {
      await this.initUsecases.installCudaToolkitDependency(options);
    }
  }
}
