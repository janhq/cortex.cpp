import { RootCommand, CommandRunner, Option } from 'nest-commander';
import { PullCommand } from './pull.command';
import { ServeCommand } from './serve.command';
import { InferenceCommand } from './inference.command';
import { ModelsCommand } from './models.command';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { defaultCortexJsHost, defaultCortexJsPort } from 'constant';

@RootCommand({
  subCommands: [ModelsCommand, PullCommand, ServeCommand, InferenceCommand],
})
export class BasicCommand extends CommandRunner {
  constructor(private readonly cortexUsecases: CortexUsecases) {
    super();
  }

  async run(input: string[], options?: any): Promise<void> {
    const command = input[0];

    switch (command) {
      case 'start':
        const host = options?.host || defaultCortexJsHost;
        const port = options?.port || defaultCortexJsPort;
        return this.cortexUsecases
          .startCortex(host, port)
          .then((e) => console.log(e));
      case 'stop':
        return this.cortexUsecases
          .stopCortex(defaultCortexJsHost, defaultCortexJsPort)
          .then((e) => console.log(e));
      default:
        console.error(`Command ${command} is not supported`);
        return;
    }
  }

  @Option({
    flags: '--host <host>',
    description: 'Host to serve the application',
  })
  parseHost(value: string) {
    return value;
  }

  @Option({
    flags: '--port <port>',
    description: 'Port to serve the application',
  })
  parsePort(value: string) {
    return parseInt(value, 10);
  }
}
