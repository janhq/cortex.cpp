import { spawn } from 'child_process';
import {
  CORTEX_JS_STOP_API_SERVER_URL,
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { join } from 'path';

type ServeOptions = {
  host?: string;
  port?: number;
  attach: boolean;
};

@SubCommand({
  name: 'serve',
  description: 'Providing API endpoint for Cortex backend',
})
export class ServeCommand extends CommandRunner {
  async run(_input: string[], options?: ServeOptions): Promise<void> {
    const host = options?.host || defaultCortexJsHost;
    const port = options?.port || defaultCortexJsPort;

    if (_input[0] === 'stop') {
      return this.stopServer().then(() => console.log('API server stopped'));
    } else {
      return this.startServer(host, port, options);
    }
  }

  private async stopServer() {
    return fetch(CORTEX_JS_STOP_API_SERVER_URL(), {
      method: 'DELETE',
    }).catch(() => {});
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
