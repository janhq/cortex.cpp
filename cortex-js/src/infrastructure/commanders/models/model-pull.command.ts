import { CommandRunner, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { SetCommandContext } from '../decorators/CommandContext';
import { ContextService } from '@/util/context.service';
import { ModelsCliUsecases } from '@commanders/usecases/models.cli.usecases';
import { ModelNotFoundException } from '@/infrastructure/exception/model-not-found.exception';

@SubCommand({
  name: 'pull',
  aliases: ['download'],
  arguments: '<model_id>',
  argsDescription: { model_id: 'Model repo to pull' },
  description: 'Download a model. Working with HuggingFace model id.',
})
@SetCommandContext()
export class ModelPullCommand extends CommandRunner {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }

  async run(passedParams: string[]) {
    if (passedParams.length < 1) {
      console.error('Model Id is required');
      exit(1);
    }

    await this.modelsCliUsecases
      .pullModel(passedParams[0])
      .catch((e: Error) => {
        if (e instanceof ModelNotFoundException)
          console.error('Model does not exist.');
        else console.error(e);
        exit(1);
      });

    console.log('\nDownload complete!');
    exit(0);
  }
}
