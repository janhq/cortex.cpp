import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { SubCommand, Option, InquirerService } from 'nest-commander';
import { exit } from 'node:process';
import ora from 'ora';
import { ChatCliUsecases } from '@commanders/usecases/chat.cli.usecases';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { existsSync } from 'fs';
import { join } from 'path';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { Engines } from '../types/engine.interface';
import { checkModelCompatibility } from '@/utils/model-check';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { BaseCommand } from '../base.command';
import { readdirSync, readFileSync } from 'node:fs';
import { load } from 'js-yaml';
import { isLocalModel, isRemoteEngine } from '@/utils/normalize-model-id';

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
  constructor(
    private readonly cortexUsecases: CortexUsecases,
    private readonly chatCliUsecases: ChatCliUsecases,
    private readonly inquirerService: InquirerService,
    private readonly fileService: FileManagerService,
    private readonly initUsecases: EnginesUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(passedParams: string[], options: RunOptions): Promise<void> {
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

    // If not exist
    // Try Pull
    if (!(await this.cortex.models.retrieve(modelId))) {
      checkingSpinner.succeed();
      await this.cortex.models.download(modelId).catch((e: Error) => {
        if (e instanceof ModelNotFoundException)
          checkingSpinner.fail('Model does not exist.');
        else checkingSpinner.fail(e.message ?? e);
        exit(1);
      });
    }

    // Second check if model is available
    const existingModel = await this.cortex.models.retrieve(modelId);
    if (!existingModel) {
      checkingSpinner.fail(`Model is not available`);
      process.exit(1);
    }
    checkingSpinner.succeed('Model found');

    const engine = existingModel.engine || Engines.llamaCPP;
    // Pull engine if not exist
    if (
      !isRemoteEngine(engine) &&
      !existsSync(join(await this.fileService.getCortexCppEnginePath(), engine))
    ) {
      await this.initUsecases.installEngine(undefined, 'latest', engine);
    }

    return this.cortexUsecases
      .startCortex()
      .then(async () =>
        this.cortex.models.start(
          modelId,
          await this.fileService.getPreset(options.preset),
        ),
      )
      .then(() => {
        if (options.chat) {
          return this.chatCliUsecases.chat(modelId, options.threadId);
        }
        return;
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
