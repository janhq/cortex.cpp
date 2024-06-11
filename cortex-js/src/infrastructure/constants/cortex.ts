export const databaseName = 'cortex';

export const databaseFile = `${databaseName}.db`;

export const defaultCortexJsHost = 'localhost';
export const defaultCortexJsPort = 1337;

export const defaultCortexCppHost = '127.0.0.1';
export const defaultCortexCppPort = 3928;
// CORTEX CPP
export const CORTEX_CPP_EMBEDDINGS_URL = (
  host: string = defaultCortexCppHost,
  port: number = defaultCortexCppPort,
) => `http://${host}:${port}/inferences/server/embedding`;

export const CORTEX_CPP_PROCESS_DESTROY_URL = (
  host: string = defaultCortexCppHost,
  port: number = defaultCortexCppPort,
) => `http://${host}:${port}/processmanager/destroy`;

export const CORTEX_CPP_HEALTH_Z_URL = (
  host: string = defaultCortexCppHost,
  port: number = defaultCortexCppPort,
) => `http://${host}:${port}/healthz`;

export const CORTEX_CPP_MODELS_URL = (
  host: string = defaultCortexCppHost,
  port: number = defaultCortexCppPort,
) => `http://${host}:${port}/inferences/server/models`;

// INITIALIZATION
export const CORTEX_RELEASES_URL =
  'https://api.github.com/repos/janhq/cortex/releases';

export const CUDA_DOWNLOAD_URL =
  'https://catalog.jan.ai/dist/cuda-dependencies/<version>/<platform>/cuda.tar.gz';
