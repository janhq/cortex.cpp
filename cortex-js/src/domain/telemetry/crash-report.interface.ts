export interface TelemetryResource {
  timestamp: number;
  osName: string;
  osVersion: string;
  appVersion: string;
  architecture: string;
}

export interface CrashReportAttributes {
  timestamp: number;
  modelId: string;
  operation: string;
  params: any;
  contextLength: number;
  tokenPerSecond: number;
}
