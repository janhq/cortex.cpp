import { Command, CommandRunner } from 'nest-commander';
import { ModelsUsecases } from 'src/usecases/models/models.usecases';
import { DownloadModelDto } from '../dtos/models/download-model.dto';

interface BasicCommandOptions {
  string?: string;
  boolean?: boolean;
  number?: number;
}

@Command({ name: 'cortex' })
export class BasicCommand extends CommandRunner {
  constructor(private readonly modelsUsecases: ModelsUsecases) {
    super();
  }

  async run(
    passedParam: string[],
    options?: BasicCommandOptions,
  ): Promise<void> {
    const downloadDtoModel: DownloadModelDto = {
      modelId: 'mistral-ins-7b-q4',
    };
    this.modelsUsecases.downloadModel(downloadDtoModel);

    return Promise.resolve();
  }
}
