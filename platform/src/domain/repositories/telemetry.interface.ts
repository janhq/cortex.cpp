import { ModelStat } from '@/infrastructure/commanders/types/model-stat.interface';
import {
  BenchmarkHardware,
  CrashReportAttributes,
  EventAttributes,
  Telemetry,
  TelemetryAnonymized,
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

  abstract sendTelemetryToServer(
    telemetryEvent: Telemetry['event'],
  ): Promise<void>;

  abstract sendEvent(
    events: EventAttributes[],
    source: TelemetrySource,
  ): Promise<void>;

  abstract getAnonymizedData(): Promise<TelemetryAnonymized | null>;

  abstract updateAnonymousData(data: TelemetryAnonymized): Promise<void>;

  abstract sendBenchmarkToServer(data: {
    hardware: BenchmarkHardware;
    results: any;
    metrics: any;
    model: any;
    sessionId: string;
  }): Promise<void>;
}
