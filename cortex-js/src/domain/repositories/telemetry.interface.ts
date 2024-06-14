import {
  CrashReportAttributes,
  Telemetry,
  TelemetrySource,
} from '../telemetry/telemetry.interface';

export abstract class TelemetryRepository {
  abstract readCrashReports(
    callback: (Telemetry: Telemetry) => void,
  ): Promise<void>;
  abstract createCrashReport(
    crashReport: CrashReportAttributes,
    source?: TelemetrySource,
  ): Promise<void>;
  abstract getLastCrashReport(): Promise<Telemetry | null>;
  abstract markLastCrashReportAsSent(): Promise<void>;
  abstract sendTelemetryToOTelCollector(
    endpoint: string,
    telemetry: Telemetry,
  ): Promise<void>;
  abstract sendTelemetryToServer(telemetry: Telemetry): Promise<void>;
}
