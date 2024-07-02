import {
  CommandRunner,
  SubCommand,
  Option,
  InquirerService,
} from 'nest-commander';
console.time('import-command-chat-cli-usecases');
import { ChatCliUsecases } from './usecases/chat.cli.usecases';
console.timeEnd('import-command-chat-cli-usecases');
console.time('import-command-chat-exit');
import { exit } from 'node:process';
console.timeEnd('import-command-chat-exit');
console.time('import-command-chat-ps-cli-usecases');
import { PSCliUsecases } from './usecases/ps.cli.usecases';
console.timeEnd('import-command-chat-ps-cli-usecases');
console.time('import-command-chat-models-usecases');
import { ModelsUsecases } from '@/usecases/models/models.usecases';
console.timeEnd('import-command-chat-models-usecases');
console.time('import-command-chat-types');
import { SetCommandContext } from './decorators/CommandContext';
console.timeEnd('import-command-chat-types');
console.time('import-command-chat-types');
import { ModelStat } from './types/model-stat.interface';
console.timeEnd('import-command-chat-types');
console.time('import-command-chat-types');
import { ContextService } from '../services/context/context.service';
console.timeEnd('import-command-chat-types');

type ChatOptions = {
  threadId?: string;
  message?: string;
  attach: boolean;
};

@SubCommand({
  name: 'chat',
  description: 'Send a chat request to a model',
  arguments: '[model_id] [message]',
  argsDescription: {
    model_id:
      'Model ID to chat with. If there is no model_id provided, it will prompt to select from running models.',
    message: 'Message to send to the model',
  },
})
@SetCommandContext()
export class ChatCommand extends CommandRunner {
  constructor(
    private readonly inquirerService: InquirerService,
    private readonly chatCliUsecases: ChatCliUsecases,
    private readonly modelsUsecases: ModelsUsecases,
    private readonly psCliUsecases: PSCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(passedParams: string[], options: ChatOptions): Promise<void> {
    let modelId = passedParams[0];
    // First attempt to get message from input or options
    // Extract input from 1 to end of array
    let message = options.message ?? passedParams.slice(1).join(' ');

    // Check for model existing
    if (!modelId || !(await this.modelsUsecases.findOne(modelId))) {
      // Model ID is not provided
      // first input might be message input
      message = passedParams.length
        ? passedParams.join(' ')
        : options.message ?? '';
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

    if (!message) options.attach = true;

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
