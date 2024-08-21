import {
  CORTEX_JS_SYSTEM_URL,
  defaultCortexCppPort,
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';
import { getApp } from './app';
import { fileManagerService } from './infrastructure/services/file-manager/file-manager.service';
import { CortexUsecases } from './usecases/cortex/cortex.usecases';

let host: string;
let port: number;
let enginePort: number;

/**
 * Start the API server
 */
export async function start(
  name?: string,
  address?: string,
  portNumber?: number,
  enginePortNumber?: number,
  dataFolder?: string,
  logPath?: string,
) {
  if (logPath) {
    fileManagerService.setLogPath(logPath);
  }
  if (name) {
    fileManagerService.setConfigProfile(name);
    const isProfileConfigExists = fileManagerService.profileConfigExists(name);
    if (!isProfileConfigExists) {
      await fileManagerService.writeConfigFile({
        ...fileManagerService.defaultConfig(),
        apiServerHost: address || defaultCortexJsHost,
        apiServerPort: port || defaultCortexJsPort,
        cortexCppPort: Number(enginePort) || defaultCortexCppPort,
      });
    }
  }
  const {
    apiServerHost: configApiServerHost,
    apiServerPort: configApiServerPort,
    cortexCppPort: configCortexCppPort,
  } = await fileManagerService.getConfig();

  host = address || configApiServerHost || defaultCortexJsHost;
  port = portNumber || configApiServerPort || defaultCortexJsPort;
  if (host === 'localhost') {
    host = '127.0.0.1';
  }
  enginePort =
    Number(enginePortNumber) || configCortexCppPort || defaultCortexCppPort;
  const dataFolderPath = dataFolder;

  return startServer(dataFolderPath, logPath);
}

async function startServer(dataFolderPath?: string, logPath?: string) {
  const config = await fileManagerService.getConfig();
  try {
    if (dataFolderPath) {
      await fileManagerService.writeConfigFile({
        ...config,
        dataFolderPath,
      });
      // load config again to create the data folder
      await fileManagerService.getConfig(dataFolderPath);
    }
    const app = await getApp(host, port);
    const cortexUsecases = await app.resolve(CortexUsecases);
    await cortexUsecases.startCortex().catch((e) => {
      throw e;
    });
    const isServerOnline = await cortexUsecases.isAPIServerOnline();
    if (isServerOnline) {
      console.log(
        `Server is already running at http://${host}:${port}. Please use 'cortex stop' to stop the server.`,
      );
    }
    await app.listen(port, host);
    await fileManagerService.writeConfigFile({
      ...config,
      apiServerHost: host,
      apiServerPort: port,
      dataFolderPath: dataFolderPath || config.dataFolderPath,
      logPath: logPath || config.logPath,
      cortexCppPort: enginePort,
    });
  } catch (e) {
    console.error(e);
    // revert the data folder path if it was set
    await fileManagerService.writeConfigFile({
      ...config,
    });
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
  });
}
