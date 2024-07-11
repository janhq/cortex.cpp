import { CommandRunner, SubCommand } from 'nest-commander';
import { SetCommandContext } from '../decorators/CommandContext';
import { CortexClient } from '../services/cortex.client';

@SubCommand({
  name: 'create',
  description: 'Create a new vector store',
  arguments: '<name>',
  argsDescription: {
    name: 'Vector store name to create',
  },
})
@SetCommandContext()
export class VectorStoresCreateCommand extends CommandRunner {
  constructor(private readonly cortex: CortexClient) {
    super();
  }

  async run(passedParams: string[]): Promise<void> {
    this.cortex.beta.vectorStores
      .create({
        name: passedParams[0],
        metadata: {
          vector_database: 'default',
          rag_extension: 'llamaindex',
        },
      })
      .then(() => console.log('Vector store created successfully.'))
      .catch((error) => console.error(error.message ?? error));
  }
}
