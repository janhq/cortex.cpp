import { CommandRunner, SubCommand } from 'nest-commander';
import { VectorStoresCreateCommand } from './vector_stores/vs-create.command';
import { VectorStoresUploadCommand } from './vector_stores/vs-upload.command';

@SubCommand({
  name: 'vector_stores',
  aliases: ['vs'],
  subCommands: [VectorStoresCreateCommand, VectorStoresUploadCommand],
  description: 'Subcommands for managing vector_stores',
})
export class VectorStoresCommand extends CommandRunner {
  async run(): Promise<void> {
    this.command?.help();
  }
}
