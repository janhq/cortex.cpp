import {
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { getApp } from './app';
import chalk from 'chalk';
import { cleanLogs } from './utils/log';

async function bootstrap() {
  const app = await getApp();
  // getting port from env
  const host = process.env.CORTEX_JS_HOST || defaultCortexJsHost;
  const port = process.env.CORTEX_JS_PORT || defaultCortexJsPort;

  try {
    // Clean logs periodically
    cleanLogs();
    await app.listen(port, host);
    console.log(chalk.blue(`Started server at http://${host}:${port}`));
    console.log(
      chalk.blue(`API Playground available at http://${host}:${port}/api`),
    );
  } catch {
    console.error(`Failed to start server. Is port ${port} in use?`);
  }
}

bootstrap();
