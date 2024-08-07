import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { SubCommand, Option, InquirerService } from 'nest-commander';
import { exit } from 'node:process';
import ora from 'ora';
import { existsSync } from 'fs';
import { join } from 'path';
import { Engines } from './types/engine.interface';
import { checkModelCompatibility, checkRequiredVersion } from '@/utils/model-check';
import { BaseCommand } from './base.command';
import { isRemoteEngine } from '@/utils/normalize-model-id';
import { ChatClient } from './services/chat-client';
import { downloadProgress } from '@/utils/download-progress';
import { DownloadType } from '@/domain/models/download.interface';
import { isLocalFile } from '@/utils/urls';
import { parse } from 'node:path';
import { printLastErrorLines } from '@/utils/logs';
import { fileManagerService } from '../services/file-manager/file-manager.service';

type RunOptions = {
  threadId?: string;
  preset?: string;
  chat?: boolean;
};

@SubCommand({
  name: 'run',
  arguments: '[model_id]',
  argsDescription: {
    model_id:
      'Model to run. If the model is not available, it will attempt to pull.',
  },
  description: 'Shortcut to start a model and chat',
})
export class RunCommand extends BaseCommand {
  chatClient: ChatClient;
  constructor(
    protected readonly cortexUsecases: CortexUsecases,
    private readonly inquirerService: InquirerService,
  ) {
    super(cortexUsecases);
  }

  async runCommand(passedParams: string[], options: RunOptions): Promise<void> {
    this.chatClient = new ChatClient(this.cortex);
    let modelId = passedParams[0];
    const checkingSpinner = ora('Checking model...').start();
    if (!modelId) {
      try {
        modelId = await this.modelInquiry();
      } catch {
        checkingSpinner.fail('Model ID is required');
        exit(1);
      }
    }

    // Check model compatibility on this machine
    await checkModelCompatibility(modelId, checkingSpinner);

    let existingModel = await this.cortex.models.retrieve(modelId);
    // If not exist
    // Try Pull
    if (!existingModel) {
      checkingSpinner.succeed();

      console.log('Downloading model...');
      await this.cortex.models.download(modelId).catch((e: Error) => {
        checkingSpinner.fail(e.message ?? e);
        exit(1);
      });
      await downloadProgress(this.cortex, modelId);
      checkingSpinner.succeed('Model downloaded');

      // Update to persisted modelId
      // TODO: Should be retrieved from the request
      if (isLocalFile(modelId)) {
        modelId = parse(modelId).name;
      }

      // Second check if model is available
      existingModel = await this.cortex.models.retrieve(modelId);
      if (!existingModel) {
        checkingSpinner.fail(`Model is not available`);
        process.exit(1);
      }
    } else {
      checkingSpinner.succeed('Model found');
    }

    const engine = existingModel.engine || Engines.llamaCPP;
    // Pull engine if not exist
    if (
      !isRemoteEngine(engine) &&
      !existsSync(
        join(await fileManagerService.getCortexCppEnginePath(), engine),
      )
    ) {
      console.log('Downloading engine...');
      await this.cortex.engines.init(engine);
      await downloadProgress(this.cortex, undefined, DownloadType.Engine);
    }
    const { version: engineVersion } = await this.cortex.engines.retrieve(engine);
    if(existingModel.engine_version && !checkRequiredVersion(existingModel.engine_version, engineVersion)) {
      console.log(`Model engine version ${existingModel.engine_version} is not compatible with engine version ${engineVersion}`);
      process.exit(1);
    }

    const startingSpinner = ora('Loading model...').start();

    return this.cortex.models
      .start(modelId, await fileManagerService.getPreset(options.preset))
      .then(() => {
        startingSpinner.succeed('Model loaded');
        if (options.chat) this.chatClient.chat(modelId, options.threadId);
        else console.log("To start a chat session, use the '--chat' flag");
      })
      .catch(async (e) => {
        startingSpinner.fail(e.message ?? e);

        printLastErrorLines(await fileManagerService.getLogPath());
      });
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

  @Option({
    flags: '-c, --chat',
    description: 'Start a chat session after starting the model',
  })
  parseChat() {
    return true;
  }

  modelInquiry = async () => {
    const { data: models } = await this.cortex.models.list();
    if (!models.length) throw 'No models found';
    const { model } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'model',
      message: 'Select a model to start:',
      choices: models.map((e) => ({
        name: e.id,
        value: e.id,
      })),
    });
    return model;
  };
}
