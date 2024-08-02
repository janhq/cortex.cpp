import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { CortexClient } from '../services/cortex.client';
import { createReadStream } from 'fs';
import { toFile } from '@cortexso/cortex.js';

@SubCommand({
  name: 'upload',
  description: 'Upload files to a vector store',
  arguments: '<name>',
  argsDescription: {
    name: 'Vector store name to uploads files to',
  },
})
@SetCommandContext()
export class VectorStoresUploadCommand extends CommandRunner {
  constructor(protected readonly cortex: CortexClient) {
    super();
  }

  async run(
    passedParams: string[],
    options: { files: string[] },
  ): Promise<void> {
    const { files } = options;
    if (!files.length) {
      console.error('No files provided to upload');
      process.exit(0);
    }

    const fileStreams = await Promise.all(
      files.map((e) => toFile(createReadStream(e))),
    );
    this.cortex.beta.vectorStores.fileBatches
      .uploadAndPoll(passedParams[0], {
        files: fileStreams,
      })
      .then(() => console.log('Files uploaded successfully'));
  }

  @Option({
    flags: '-f, --files <files>',
    description: 'Files to upload to the vector store',
  })
  parseFiles(value: string[]) {
    if (!Array.isArray(value)) return [value];
    return value;
  }
}
