import { SubCommand, Option, InquirerService } from 'nest-commander';
import ora from 'ora';
import { exit } from 'node:process';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { existsSync } from 'node:fs';
import { fileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { join } from 'node:path';
import { Engines } from '../types/engine.interface';
import { checkModelCompatibility } from '@/utils/model-check';
import { BaseCommand } from '../base.command';
import { isRemoteEngine } from '@/utils/normalize-model-id';
import { downloadProgress } from '@/utils/download-progress';
import { DownloadType } from '@/domain/models/download.interface';
import { printLastErrorLines } from '@/utils/logs';
import { CortexClient } from '../services/cortex.client';

type ModelStartOptions = {
  preset?: string;
};

@SubCommand({
  name: 'start',
  description: 'Start a model by ID.',
  arguments: '[model_id]',
  argsDescription: {
    model_id:
      'Model ID to start. If there is no model ID, it will prompt you to select from the available models.',
  },
})
@SetCommandContext()
export class ModelStartCommand extends BaseCommand {
  constructor(
    private readonly inquirerService: InquirerService,
    readonly cortexUsecases: CortexUsecases,
    readonly contextService: ContextService,
    readonly cortex: CortexClient,
  ) {
    super(cortexUsecases);
  }

  async runCommand(
    passedParams: string[],
    options: ModelStartOptions,
  ): Promise<void> {
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

    const existingModel = await this.cortex.models.retrieve(modelId);
    if (!existingModel) {
      checkingSpinner.fail(
        `Model ${modelId} not found on filesystem.\nPlease try 'cortex pull ${modelId}' first.`,
      );
      process.exit(1);
    }

    await checkModelCompatibility(modelId);
    checkingSpinner.succeed('Model found');

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

    const parsedPreset = await fileManagerService.getPreset(options.preset);

    const startingSpinner = ora('Loading model...').start();

    await this.cortex.models
      .start(modelId, parsedPreset)
      .then(() => startingSpinner.succeed('Model loaded'))
      .catch(async (error) => {
        startingSpinner.fail(error.message ?? error);
        printLastErrorLines(await fileManagerService.getLogPath());
      });
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

  @Option({
    flags: '-p, --preset <preset>',
    description: 'Apply a chat preset to the chat session',
  })
  parseTemplate(value: string) {
    return value;
  }
}
