// Type definitions for cpuinfo
// Project: http://github.com/tryghost/node-sqlite3

/// <reference types="node" />
export type InstructionSet = 'AVX' | 'AVX2' | 'AVX512'; // Add more as needed later
declare module "cpuinfo" {
  declare const cpuInfo: {
    cpuInfo: () => InstructionSet[];
  }
}