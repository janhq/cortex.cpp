import {
  CommandRunner,
  SubCommand,
  Option,
  InquirerService,
} from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

type ModelStartOptions = {
  attach: boolean;
};
@SubCommand({ name: 'start', description: 'Start a model by ID.' })
export class ModelStartCommand extends CommandRunner {
  constructor(
    private readonly inquirerService: InquirerService,
    private readonly cortexUsecases: CortexUsecases,
    private readonly modelsCliUsecases: ModelsCliUsecases,
  ) {
    super();
  }

  async run(input: string[], options: ModelStartOptions): Promise<void> {
    let modelId = input[0];
    if (!modelId) {
      try {
        modelId = await this.modelInquiry();
      } catch {
        console.error('Model ID is required');
        exit(1);
      }
    }

    await this.cortexUsecases
      .startCortex(options.attach)
      .then(() => this.modelsCliUsecases.startModel(input[0]))
      .then(console.log)
      .then(() => !options.attach && process.exit(0));
  }

  modelInquiry = async () => {
    const models = await this.modelsCliUsecases.listAllModels();
    if (!models.length) throw 'No models found';
    const { model } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'model',
      message: 'Select a model to start:',
      choices: models.map((e) => ({
        name: e.name,
        value: e.id,
      })),
    });
    return model;
  };

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
