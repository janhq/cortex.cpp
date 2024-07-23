import { SubCommand } from 'nest-commander';
import { exit } from 'node:process';
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
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { checkModelCompatibility } from '@/utils/model-check';
import { Engines } from '../types/engine.interface';
import { EnginesUsecases } from '@/usecases/engines/engines.usecase';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';

@SubCommand({
  name: 'pull',
  aliases: ['download'],
  arguments: '<model_id>',
  argsDescription: { model_id: 'Model repo to pull' },
  description:
    'Download a model from a registry. Working with HuggingFace repositories. For available models, please visit https://huggingface.co/cortexso',
})
@SetCommandContext()
export class ModelPullCommand extends BaseCommand {
  constructor(
    private readonly engineUsecases: EnginesUsecases,
    private readonly fileService: FileManagerService,
    readonly contextService: ContextService,
    private readonly telemetryUsecases: TelemetryUsecases,
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

    // TODO: support pull from cortexjs
    await this.cortex.models.download(modelId).catch((e: Error) => {
      if (e instanceof ModelNotFoundException)
        console.error('Model does not exist.');
      else console.error(e.message ?? e);
      exit(1);
    });

    const existingModel = await this.cortex.models.retrieve(modelId);
    const engine = existingModel?.engine || Engines.llamaCPP;

    // Pull engine if not exist
    if (
      !existsSync(join(await this.fileService.getCortexCppEnginePath(), engine))
    ) {
      console.log('\n');
      // TODO: support install engine from cortexjs
      await this.engineUsecases.installEngine(undefined, 'latest', engine);
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
