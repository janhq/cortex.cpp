import { exit, stdin, stdout } from 'node:process';
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
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { checkModelCompatibility } from '@/utils/model-check';
import { Engines } from '../types/engine.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from '../base.command';
import { Presets, SingleBar } from 'cli-progress';

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
    if(await this.cortex.models.retrieve(modelId)){
      console.error('Model already exists.');
      exit(1);
    }

    await this.cortex.models.download(modelId).catch((e: Error) => {
      if (e instanceof ModelNotFoundException)
        console.error('Model does not exist.');
      else console.error(e.message ?? e);
      exit(1);
    });
    
    const response = await this.cortex.models.downloadEvent()
    
    const  rl = require("readline").createInterface({
      input: stdin,
      output: stdout,
    });

    rl.on('SIGINT', () => {
      console.log('\nStopping download...');
      process.emit("SIGINT");
    }); 
    process.on('SIGINT', async() => {
      await this.cortex.models.abortDownload(modelId);
      exit(1);
    });

    const progressBar = new SingleBar({}, Presets.shades_classic);
    progressBar.start(100, 0);

    const readableStream = response.toReadableStream()
    const decoder = new TextDecoder();
    for await (const stream of readableStream) {
      const part = JSON.parse(decoder.decode(stream))
      console.log(part, );
      if(part.length){
        const data = part[0] as any
        let totalBytes = 0;
        let totalTransferred = 0;
          data.children.forEach((child: any) => {
            totalBytes+=child.size.total
            totalTransferred+=child.size.transferred
          })
      progressBar.update(Math.floor((totalTransferred / totalBytes) * 100))
      }
    }
    rl.close();

    const existingModel = await this.cortex.models.retrieve(modelId);
    const engine = existingModel?.engine || Engines.llamaCPP;

    // Pull engine if not exist
    if (
      !existsSync(join(await this.fileService.getCortexCppEnginePath(), engine))
    ) {
      console.log('\n');
      await this.cortex.engines.init(engine);
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

  private async abortDownload(modelId: string) {

  }
}
