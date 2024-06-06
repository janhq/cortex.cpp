import { defaultCortexCppHost, defaultCortexCppPort } from '@/../constant';

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

// INITIALIZATION
export const CORTEX_RELEASES_URL =
  'https://api.github.com/repos/janhq/cortex/releases';

export const CUDA_DOWNLOAD_URL =
  'https://catalog.jan.ai/dist/cuda-dependencies/<version>/<platform>/cuda.tar.gz';
