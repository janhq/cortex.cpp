import { CommandRunner, SubCommand } from 'nest-commander';
import { Presets, SingleBar } from 'cli-progress';
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

    const bar = new SingleBar({}, Presets.shades_classic);
    bar.start(100, 0);
    const callback = (progress: number) => {
      bar.update(progress);
    };
    await this.modelsCliUsecases.pullModel(input[0], callback);
    console.log('\nDownload complete!');
    exit(0);
  }
}
