import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import {
  CommandRunner,
  SubCommand,
  Option,
  InquirerService,
} from 'nest-commander';
import { exit } from 'node:process';
import { ChatCliUsecases } from '../usecases/chat.cli.usecases';
import { defaultCortexCppHost, defaultCortexCppPort } from 'constant';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';

type RunOptions = {
  threadId?: string;
  preset?: string;
};

@SubCommand({
  name: 'run',
  description: 'EXPERIMENTAL: Shortcut to start a model and chat',
})
export class RunCommand extends CommandRunner {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly chatCliUsecases: ChatCliUsecases,
    private readonly inquirerService: InquirerService,
  ) {
    super();
  }

  async run(input: string[], options: RunOptions): Promise<void> {
    let modelId = input[0];
    if (!modelId) {
      try {
        modelId = await this.modelInquiry();
      } catch {
        console.error('Model ID is required');
        exit(1);
      }
    }

    return this.cortexUsecases
      .startCortex(false, defaultCortexCppHost, defaultCortexCppPort)
      .then(() => this.modelsCliUsecases.startModel(modelId, options.preset))
      .then(() => this.chatCliUsecases.chat(modelId, options.threadId));
  }

  @Option({
    flags: '-t, --thread <thread_id>',
    description: 'Thread Id. If not provided, will create new thread',
  })
  parseThreadId(value: string) {
    return value;
  }

  @Option({
    flags: '-p, --preset <preset>',
    description: 'Apply a chat preset to the chat session',
  })
  parseTemplate(value: string) {
    return value;
  }

  modelInquiry = async () => {
    const models = (await this.modelsCliUsecases.listAllModels()).filter(
      (model) =>
        Array.isArray(model.files) &&
        !/^(http|https):\/\/[^/]+\/.*/.test(model.files[0]),
    );
    if (!models.length) throw 'No models found';
    const { model } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'model',
      message: 'Select a model to start:',
      choices: models.map((e) => ({
        name: e.name,
        value: e.model,
      })),
    });
    return model;
  };
}
