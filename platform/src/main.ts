import {
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { getApp } from './app';

process.title = 'Cortex API Server';

async function bootstrap() {
  // getting port from env
  const host = process.env.CORTEX_JS_HOST || defaultCortexJsHost;
  const port = process.env.CORTEX_JS_PORT || defaultCortexJsPort;
  const app = await getApp(host, Number(port));
  try {
    await app.listen(port, host);
    console.log(`Started server at http://${host}:${port}`);
    console.log(`API Playground available at http://${host}:${port}/api`);
  } catch (error) {
    console.error(`Failed to start server. Is port ${port} in use? ${error}`);
  }
}

bootstrap();
