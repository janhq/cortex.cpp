import {
  CORTEX_JS_STOP_API_SERVER_URL,
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { getApp } from './app';
import chalk from 'chalk';

/**
 * Start the API server
 */
export async function start(host?: string, port?: number) {
  const app = await getApp();
  // getting port from env
  const sHost = host || process.env.CORTEX_JS_HOST || defaultCortexJsHost;
  const sPort = port || process.env.CORTEX_JS_PORT || defaultCortexJsPort;

  try {
    await app.listen(sPort, sHost);
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
export async function stop() {
  return fetch(CORTEX_JS_STOP_API_SERVER_URL(), {
    method: 'DELETE',
  }).catch(() => {});
}
