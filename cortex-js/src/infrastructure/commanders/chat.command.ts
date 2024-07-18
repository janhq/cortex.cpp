import {
  CommandRunner,
  SubCommand,
  Option,
  InquirerService,
} from 'nest-commander';
import ora from 'ora';
import { ChatCliUsecases } from './usecases/chat.cli.usecases';
import { exit } from 'node:process';
import { PSCliUsecases } from './usecases/ps.cli.usecases';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { SetCommandContext } from './decorators/CommandContext';
import { ModelStat } from './types/model-stat.interface';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import {
  EventName,
  TelemetrySource,
} from '@/domain/telemetry/telemetry.interface';
import { ContextService } from '../services/context/context.service';
import { BaseCommand } from './base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

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
export class ChatCommand extends BaseCommand {
  constructor(
    private readonly inquirerService: InquirerService,
    private readonly chatCliUsecases: ChatCliUsecases,
    private readonly modelsUsecases: ModelsUsecases,
    private readonly psCliUsecases: PSCliUsecases,
    readonly contextService: ContextService,
    private readonly telemetryUsecases: TelemetryUsecases,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(
    passedParams: string[],
    options: ChatOptions,
  ): Promise<void> {
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
        : (options.message ?? '');
      // If model ID is not provided, prompt user to select from running models
      const models = await this.psCliUsecases.getModels();
      if (models.length === 1) {
        modelId = models[0].modelId;
      } else if (models.length > 0) {
        modelId = await this.modelInquiry(models);
      } else {
        exit(1);
      }
    }

    if (!message) options.attach = true;
    const result = await this.chatCliUsecases.chat(
      modelId,
      options.threadId,
      message, // Accept both message from inputs or arguments
      options.attach,
      false, // Do not stop cortex session or loaded model
    );
    this.telemetryUsecases.sendEvent(
      [
        {
          name: EventName.CHAT,
          modelId,
        },
      ],
      TelemetrySource.CLI,
    );
    return result;
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
