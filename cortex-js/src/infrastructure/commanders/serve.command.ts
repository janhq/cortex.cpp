import { spawn } from 'child_process';
import {
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { join } from 'path';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '@/util/context.service';
import { ServeStopCommand } from './sub-commands/serve-stop.command';

type ServeOptions = {
  address?: string;
  port?: number;
  attach: boolean;
};

@SubCommand({
  name: 'serve',
  description: 'Providing API endpoint for Cortex backend',
  subCommands: [ServeStopCommand],
})
@SetCommandContext()
export class ServeCommand extends CommandRunner {
  constructor(readonly contextService: ContextService) {
    super();
  }

  async run(passedParams: string[], options?: ServeOptions): Promise<void> {
    const host = options?.address || defaultCortexJsHost;
    const port = options?.port || defaultCortexJsPort;

    return this.startServer(host, port, options);
  }

  private async startServer(
    host: string,
    port: number,
    options: ServeOptions = { attach: true },
  ) {
    const serveProcess = spawn(
      'node',
      process.env.TEST
        ? [join(__dirname, '../../../dist/src/main.js')]
        : [join(__dirname, '../../main.js')],
      {
        env: {
          ...process.env,
          CORTEX_JS_HOST: host,
          CORTEX_JS_PORT: port.toString(),
          NODE_ENV: 'production',
        },
        stdio: options?.attach ? 'inherit' : 'ignore',
        detached: true,
      },
    );
    if (!options?.attach) {
      serveProcess.unref();
      console.log('Started server at http://%s:%d', host, port);
    }
  }

  @Option({
    flags: '-a, --address <address>',
    description: 'Address to use',
  })
  parseHost(value: string) {
    return value;
  }

  @Option({
    flags: '-p, --port <port>',
    description: 'Port to serve the application',
  })
  parsePort(value: string) {
    return parseInt(value, 10);
  }

  @Option({
    flags: '-a, --attach',
    description: 'Attach to interactive chat session',
    defaultValue: false,
    name: 'attach',
  })
  parseAttach() {
    return true;
  }
}
