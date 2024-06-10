import { spawn } from 'child_process';
import {
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { join } from 'path';

type ServeOptions = {
  host?: string;
  port?: number;
};

@SubCommand({
  name: 'serve',
  description: 'Providing API endpoint for Cortex backend',
})
export class ServeCommand extends CommandRunner {
  async run(_input: string[], options?: ServeOptions): Promise<void> {
    const host = options?.host || defaultCortexJsHost;
    const port = options?.port || defaultCortexJsPort;

    spawn(
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
        stdio: 'inherit',
        detached: false,
      },
    );
  }

  @Option({
    flags: '-h, --host <host>',
    description: 'Host to serve the application',
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
}
