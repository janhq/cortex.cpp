import { exit } from 'node:process';
import { SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import {
  EventName,
  TelemetrySource,
} from '@/domain/telemetry/telemetry.interface';
import { ContextService } from '@/infrastructure/services/context/context.service';
import { existsSync } from 'fs';
import { join } from 'node:path';
import { checkModelCompatibility } from '@/utils/model-check';
import { Engines } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';
import { downloadProgress } from '@/utils/download-progress';
import { DownloadType } from '@/domain/models/download.interface';
import ora from 'ora';
import { isRemoteEngine } from '@/utils/normalize-model-id';
import { fileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

@SubCommand({
  name: 'pull',
  aliases: ['download'],
  arguments: '<model_id>',
  argsDescription: { model_id: 'Model repo to pull' },
  description:
    'Download a model by URL (or HuggingFace ID). See built-in models: https://huggingface.co/cortexso',
})
@SetCommandContext()
export class ModelPullCommand extends BaseCommand {
  constructor(
    private readonly telemetryUsecases: TelemetryUsecases,
    readonly contextService: ContextService,
    readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }

  async runCommand(passedParams: string[]) {
    if (passedParams.length < 1) {
      console.error('Model Id is required');
      exit(1);
    }
    const modelId = passedParams[0];

    await checkModelCompatibility(modelId);
    if (await this.cortex.models.retrieve(modelId)) {
      console.error('Model already exists.');
      exit(1);
    }

    console.log('Downloading model...');
    await this.cortex.models.download(modelId).catch((e: Error) => {
      if (e instanceof ModelNotFoundException)
        console.error('Model does not exist.');
      else console.error(e.message ?? e);
      exit(1);
    });

    await downloadProgress(this.cortex, modelId);
    ora().succeed('Model downloaded');

    const existingModel = await this.cortex.models.retrieve(modelId);
    const engine = existingModel?.engine || Engines.llamaCPP;

    // Pull engine if not exist
    if (
      !isRemoteEngine(engine) &&
      !existsSync(
        join(await fileManagerService.getCortexCppEnginePath(), engine),
      )
    ) {
      console.log('\n');
      console.log('Downloading engine...');
      await this.cortex.engines.init(engine);
      await downloadProgress(this.cortex, undefined, DownloadType.Engine);
    }
    this.telemetryUsecases.sendEvent(
      [
        {
          name: EventName.DOWNLOAD_MODEL,
          modelId: passedParams[0],
        },
      ],
      TelemetrySource.CLI,
    );
    console.log('\nDownload complete!');

    exit(0);
  }
}
