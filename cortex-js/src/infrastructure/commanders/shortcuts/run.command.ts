import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import {
  CommandRunner,
  SubCommand,
  Option,
  InquirerService,
} from 'nest-commander';
import { exit } from 'node:process';
import ora from 'ora';
import { ChatCliUsecases } from '@commanders/usecases/chat.cli.usecases';
import { ModelsCliUsecases } from '@commanders/usecases/models.cli.usecases';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { existsSync } from 'fs';
import { join } from 'path';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { InitCliUsecases } from '../usecases/init.cli.usecases';
import { Engines } from '../types/engine.interface';
import { checkModelCompatibility } from '@/utils/model-check';

type RunOptions = {
  threadId?: string;
  preset?: string;
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
export class RunCommand extends CommandRunner {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly chatCliUsecases: ChatCliUsecases,
    private readonly inquirerService: InquirerService,
    private readonly fileService: FileManagerService,
    private readonly initUsecases: InitCliUsecases,
  ) {
    super();
  }

  async run(passedParams: string[], options: RunOptions): Promise<void> {
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
    await checkModelCompatibility(modelId);

    // If not exist
    // Try Pull
    if (!(await this.modelsCliUsecases.getModel(modelId))) {
      checkingSpinner.succeed('Model not found. Attempting to pull...');
      await this.modelsCliUsecases.pullModel(modelId).catch((e: Error) => {
        if (e instanceof ModelNotFoundException)
          checkingSpinner.fail('Model does not exist.');
        else checkingSpinner.fail(e.message ?? e);
        exit(1);
      });
    }

    // Second check if model is available
    const existingModel = await this.modelsCliUsecases.getModel(modelId);
    if (
      !existingModel ||
      !Array.isArray(existingModel.files) ||
      /^(http|https):\/\/[^/]+\/.*/.test(existingModel.files[0])
    ) {
      checkingSpinner.fail(`Model is not available`);
      process.exit(1);
    }
    checkingSpinner.succeed('Model found');

    const engine = existingModel.engine || Engines.llamaCPP;
    // Pull engine if not exist
    if (
      !existsSync(join(await this.fileService.getCortexCppEnginePath(), engine))
    ) {
      const engineSpinner = ora('Installing engine...').start();
      await this.initUsecases.installEngine(
        await this.initUsecases.defaultInstallationOptions(),
        'latest',
        engine,
      );
      engineSpinner.succeed('Engine installed');
    }

    return this.cortexUsecases
      .startCortex(false)
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
    const models = await this.modelsCliUsecases.listAllModels();
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
