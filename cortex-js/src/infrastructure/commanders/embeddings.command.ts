import { InquirerService, Option, SubCommand } from 'nest-commander';
import { inspect } from 'util';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from './base.command';
import { Cortex } from '@cortexso/cortex.js';
import ora from 'ora';
import { FileManagerService } from '../services/file-manager/file-manager.service';

interface EmbeddingCommandOptions {
  encoding_format?: string;
  input?: string;
  dimensions?: number;
}

@SubCommand({
  name: 'embeddings',
  arguments: '[model_id]',
  description: 'Creates an embedding vector representing the input text.',
  argsDescription: {
    model_id:
      'Model to use for embedding. If not provided, it will prompt to select from running models.',
  },
})
export class EmbeddingCommand extends BaseCommand {
  constructor(
    private readonly inquirerService: InquirerService,
    readonly cortexUsecases: CortexUsecases,
    private readonly fileService: FileManagerService,
  ) {
    super(cortexUsecases, fileService);
  }
  async runCommand(
    passedParams: string[],
    options: EmbeddingCommandOptions,
  ): Promise<void> {
    let model = passedParams[0];
    const checkingSpinner = ora('Checking model...').start();
    // First attempt to get message from input or options
    let input: string | string[] = options.input ?? passedParams.splice(1);

    // Check for model existing
    if (!model || !(await this.cortex.models.retrieve(model))) {
      // Model ID is not provided
      // first input might be message input
      input = passedParams ?? options.input;
      // If model ID is not provided, prompt user to select from running models
      const { data: models } = await this.cortex.models.list();
      if (models.length === 1) {
        model = models[0].id;
      } else if (models.length > 0) {
        model = await this.modelInquiry(models);
      } else {
        checkingSpinner.fail('Model ID is required');
        process.exit(1);
      }
    }
    checkingSpinner.succeed(`Model found`);
    return this.cortex.embeddings
      .create({
        input,
        model,
      })
      .then((res) =>
        inspect(res, { showHidden: false, depth: null, colors: true }),
      )
      .then(console.log)
      .catch((e) => console.error(e.message ?? e));
  }

  modelInquiry = async (models: Cortex.Model[]) => {
    const { model } = await this.inquirerService.inquirer.prompt({
      type: 'list',
      name: 'model',
      message: 'Select model to chat with:',
      choices: models.map((e) => ({
        name: e.id,
        value: e.id,
      })),
    });
    return model;
  };

  @Option({
    flags: '-i, --input <input>',
    description:
      'Input text to embed, encoded as a string or array of tokens. To embed multiple inputs in a single request, pass an array of strings or array of token arrays.',
  })
  parseInput(value: string) {
    return value;
  }

  @Option({
    flags: '-e, --encoding_format <encoding_format>',
    description:
      'Encoding format for the embeddings. Supported formats are float and int.',
  })
  parseEncodingFormat(value: string) {
    return value;
  }

  @Option({
    flags: '-d, --dimensions <dimensions>',
    description:
      'The number of dimensions the resulting output embeddings should have. Only supported in some models.',
  })
  parseDimensionsFormat(value: string) {
    return value;
  }
}
