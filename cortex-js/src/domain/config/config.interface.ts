export interface Config {
  dataFolderPath: string;
  cortexCppHost: string;
  cortexCppPort: number;
  // todo: will remove optional when all command request api server
  apiServerPort?: number;
  apiServerHost?: string;
}
