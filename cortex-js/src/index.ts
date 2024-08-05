import {
  CORTEX_CPP_PROCESS_DESTROY_URL,
  CORTEX_JS_SYSTEM_URL,
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { getApp } from './app';
import chalk from 'chalk';
import { CortexUsecases } from './usecases/cortex/cortex.usecases';
import { cleanLogs } from './utils/log';

/**
 * Start the API server
 */
export async function start(host?: string, port?: number) {
  const app = await getApp(host, port);
  // getting port from env
  const sHost = host || process.env.CORTEX_JS_HOST || defaultCortexJsHost;
  const sPort = port || process.env.CORTEX_JS_PORT || defaultCortexJsPort;

  try {
    // Clean log periodically
    cleanLogs();
    await app.listen(sPort, sHost);
    const cortexUsecases = await app.resolve(CortexUsecases);
    await cortexUsecases.startCortex();
    console.log(chalk.blue(`Started server at http://${sHost}:${sPort}`));
    console.log(
      chalk.blue(`API Playground available at http://${sHost}:${sPort}/api`),
    );
  } catch {
    console.error(`Failed to start server. Is port ${port} in use?`);
  }
}

/**
 * Stop the API server
 * @returns
 */
export async function stop(host?: string, port?: number) {
  return fetch(CORTEX_JS_SYSTEM_URL(host, port), {
    method: 'DELETE',
  })
    .catch(() => {})
    .then(() =>
      fetch(CORTEX_CPP_PROCESS_DESTROY_URL(host, port), {
        method: 'DELETE',
      }),
    )
    .catch(() => {});
}
