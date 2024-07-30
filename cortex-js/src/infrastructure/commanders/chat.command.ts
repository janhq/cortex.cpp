import { existsSync } from 'fs';
import { SubCommand, Option, InquirerService } from 'nest-commander';
import { exit } from 'node:process';
import { SetCommandContext } from './decorators/CommandContext';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import {
  EventName,
  TelemetrySource,
} from '@/domain/telemetry/telemetry.interface';
import { ContextService } from '../services/context/context.service';
import { BaseCommand } from './base.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { Engines } from './types/engine.interface';
import { join } from 'path';
import { FileManagerService } from '../services/file-manager/file-manager.service';
import { isRemoteEngine } from '@/utils/normalize-model-id';
import { Cortex } from '@cortexso/cortex.js';
import { ChatClient } from './services/chat-client';
import { downloadProgress } from '@/utils/download-progress';
import { CortexClient } from './services/cortex.client';
import { DownloadType } from '@/domain/models/download.interface';

type ChatOptions = {
  threadId?: string;
  message?: string;
  attach: boolean;
  preset?: string;
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
  chatClient: ChatClient;

  constructor(
    private readonly inquirerService: InquirerService,
    private readonly telemetryUsecases: TelemetryUsecases,
    private readonly fileService: FileManagerService,
    protected readonly cortexUsecases: CortexUsecases,
    protected readonly contextService: ContextService,
    protected readonly cortex: CortexClient,
  ) {
    super(cortexUsecases);
    this.chatClient = new ChatClient(this.cortex);
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
    if (!modelId || !(await this.cortex.models.retrieve(modelId))) {
      // Model ID is not provided
      // first input might be message input
      message = passedParams.length
        ? passedParams.join(' ')
        : (options.message ?? '');
      // If model ID is not provided, prompt user to select from running models
      const { data: models } = await this.cortex.models.list();
      if (models.length === 1) {
        modelId = models[0].id;
      } else if (models.length > 0) {
        modelId = await this.modelInquiry(models);
      } else {
        exit(1);
      }
    }

    const existingModel = await this.cortex.models.retrieve(modelId);
    if (!existingModel) {
      process.exit(1);
    }

    const engine = existingModel.engine || Engines.llamaCPP;
    // Pull engine if not exist
    if (
      !isRemoteEngine(engine) &&
      !existsSync(join(await this.fileService.getCortexCppEnginePath(), engine))
    ) {
      console.log('Downloading engine...');
      await this.cortex.engines.init(engine);
      await downloadProgress(this.cortex, undefined, DownloadType.Engine);
    }

    if (!message) options.attach = true;
    this.telemetryUsecases.sendEvent(
      [
        {
          name: EventName.CHAT,
          modelId,
        },
      ],
      TelemetrySource.CLI,
    );

    const preset = await this.fileService.getPreset(options.preset);

    return this.chatClient.chat(
      modelId,
      options.threadId,
      message, // Accept both message from inputs or arguments
      preset ? preset : {},
    );
  }

  modelInquiry = async (models: Cortex.Model[]) => {
    const { model } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'model',
      message: 'Select a model to chat with:',
      choices: models.map((e) => ({
        name: e.id,
        value: e.id,
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

  @Option({
    flags: '-p, --preset <preset>',
    description: 'Apply a chat preset to the chat session',
  })
  parsePreset(value: string) {
    return value;
  }
}
