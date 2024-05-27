import { CommandRunner, SubCommand } from 'nest-commander';
import { exit } from 'node:process';
import { ModelsCliUsecases } from '../usecases/models.cli.usecases';

@SubCommand({
  name: 'pull',
  aliases: ['download'],
  description: 'Download a model. Working with HuggingFace model id.',
})
export class ModelPullCommand extends CommandRunner {
  constructor(private readonly modelsCliUsecases: ModelsCliUsecases) {
    super();
  }

  async run(input: string[]) {
    if (input.length < 1) {
      console.error('Model ID is required');
      exit(1);
    }

    await this.modelsCliUsecases.pullModel(input[0]);
    console.log('\nDownload complete!');
    exit(0);
  }
}
