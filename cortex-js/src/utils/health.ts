import { CORTEX_JS_HEALTH_URL_WITH_API_PATH } from '@/infrastructure/constants/cortex';

/**
 * Check whether the Cortex CPP is healthy
 * @param host
 * @param port
 * @returns
 */
export function healthCheck(apiPath: string): Promise<boolean> {
  return fetch(CORTEX_JS_HEALTH_URL_WITH_API_PATH(apiPath))
    .then((res) => {
      if (res.ok) {
        return true;
      }
      return false;
    })
    .catch(() => false);
}

/**
 * Wait until the Cortex Server is healthy
 * @param host
 * @param port
 * @returns
 */
export function waitUntilHealthy(apiPath: string): Promise<boolean> {
  return new Promise((resolve) => {
    const interval = setInterval(async () => {
      const isHealthy = await healthCheck(apiPath);
      if (isHealthy) {
        clearInterval(interval);
        resolve(true);
      }
    }, 1000);
  });
}
