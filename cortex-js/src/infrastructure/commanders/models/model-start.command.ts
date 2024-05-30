import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

type ModelStartOptions = {
  attach: boolean;
};
@SubCommand({ name: 'start', description: 'Start a model by ID.' })
export class ModelStartCommand extends CommandRunner {
  constructor(
    private readonly cortexUsecases: CortexUsecases,
    private readonly modelsCliUsecases: ModelsCliUsecases,
  ) {
    super();
  }

  async run(input: string[], options: ModelStartOptions): Promise<void> {
    if (input.length === 0) {
      console.error('Model ID is required');
      exit(1);
    }

    await this.cortexUsecases
      .startCortex(options.attach)
      .then(() => this.modelsCliUsecases.startModel(input[0]))
      .then(console.log)
      .then(() => !options.attach && process.exit(0));
  }

  @Option({
    flags: '-a, --attach',
    description: 'Attach to interactive chat session',
    defaultValue: false,
    name: 'attach',
  })
  parseAttach() {
    return true;
  }
}
