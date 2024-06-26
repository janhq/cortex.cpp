import {
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { getApp } from './app';
import chalk from 'chalk';

async function bootstrap() {
  const app = await getApp();
  // getting port from env
  const host = process.env.CORTEX_JS_HOST || defaultCortexJsHost;
  const port = process.env.CORTEX_JS_PORT || defaultCortexJsPort;

  try {
    await app.listen(port, host);
    console.log(chalk.blue(`Started server at http://${host}:${port}`));
    console.log(
      chalk.blue(`API Playground available at http://${host}:${port}/api`),
    );
  } catch (err) {
    console.error(err.message);
  }
}

bootstrap();
