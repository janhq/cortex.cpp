import {
  CommandRunner,
  SubCommand,
  Option,
  InquirerService,
} from 'nest-commander';
import { ChatCliUsecases } from './usecases/chat.cli.usecases';
import { exit } from 'node:process';
import { ModelStat, PSCliUsecases } from './usecases/ps.cli.usecases';
import { ModelsUsecases } from '@/usecases/models/models.usecases';

type ChatOptions = {
  threadId?: string;
  message?: string;
  attach: boolean;
};

@SubCommand({ name: 'chat', description: 'Send a chat request to a model' })
export class ChatCommand extends CommandRunner {
  constructor(
    private readonly inquirerService: InquirerService,
    private readonly chatCliUsecases: ChatCliUsecases,
    private readonly modelsUsecases: ModelsUsecases,
    private readonly psCliUsecases: PSCliUsecases,
  ) {
    super();
  }

  async run(_input: string[], options: ChatOptions): Promise<void> {
    let modelId = _input[0];
    // First attempt to get message from input or options
    let message = _input[1] ?? options.message;

    // Check for model existing
    if (!modelId || !(await this.modelsUsecases.findOne(modelId))) {
      // Model ID is not provided
      // first input might be message input
      message = _input[0] ?? options.message;
      // If model ID is not provided, prompt user to select from running models
      const models = await this.psCliUsecases.getModels();
      if (models.length === 1) {
        modelId = models[0].modelId;
      } else if (models.length > 0) {
        modelId = await this.modelInquiry(models);
      } else {
        console.error('Model ID is required');
        exit(1);
      }
    }

    return this.chatCliUsecases.chat(
      modelId,
      options.threadId,
      message, // Accept both message from inputs or arguments
      options.attach,
      false, // Do not stop cortex session or loaded model
    );
  }

  modelInquiry = async (models: ModelStat[]) => {
    const { model } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'model',
      message: 'Select running model to chat with:',
      choices: models.map((e) => ({
        name: e.modelId,
        value: e.modelId,
      })),
    });
    return model;
  };

  @Option({
    flags: '-t, --thread <thread_id>',
    description: 'Thread Id. If not provided, will create new thread',
  })
  parseThreadId(value: string) {
    return value;
  }

  @Option({
    flags: '-m, --message <message>',
    description: 'Message to send to the model',
  })
  parseModelId(value: string) {
    return value;
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
