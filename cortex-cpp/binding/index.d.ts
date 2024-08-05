// Type definitions for cortex-cpp node binding

/// <reference types="node" />
declare module "cortex-cpp" {
  export function start(port?: number);
  export function stop();
}
