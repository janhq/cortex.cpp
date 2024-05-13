import { spawn } from 'child_process';
import { defaultCortexJsHost, defaultCortexJsPort } from 'constant';
import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { join } from 'path';

type ServeOptions = {
  host?: string;
  port?: number;
};

@SubCommand({ name: 'serve' })
export class ServeCommand extends CommandRunner {
  constructor() {
    super();
  }

  async run(_input: string[], options?: ServeOptions): Promise<void> {
    const host = options?.host || defaultCortexJsHost;
    const port = options?.port || defaultCortexJsPort;

    spawn('node', [join(__dirname, '../../main.js')], {
      env: {
        ...process.env,
        CORTEX_JS_HOST: host,
        CORTEX_JS_PORT: port.toString(),
        NODE_ENV: 'production',
      },
      stdio: 'inherit',
      detached: false,
    });
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
