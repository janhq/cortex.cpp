import { TelemetryRepository } from '@/domain/repositories/telemetry.interface';
import {
  CrashReportAttributes,
  EventAttributes,
  EventName,
  Telemetry,
  TelemetryAnonymized,
  TelemetrySource,
} from '@/domain/telemetry/telemetry.interface';
import { ContextService } from '@/util/context.service';
import { HttpException, Inject, Injectable, Scope } from '@nestjs/common';
import { v4 } from 'uuid';

@Injectable({ scope: Scope.TRANSIENT })
export class TelemetryUsecases {
  private readonly crashReports: string[] = [];
  private readonly maxSize = 10;
  private metricQueue: EventAttributes[] = [];
  private readonly maxQueueSize = 10;
  private readonly flushInterval = 1000 * 60 * 5;
  private interval: NodeJS.Timeout = this.flushMetricQueueInterval();
  private lastActiveAt?: string | null;

  constructor(
    @Inject('TELEMETRY_REPOSITORY')
    private readonly telemetryRepository: TelemetryRepository,
    private readonly contextService: ContextService,
  ) {
    this.catchException();
  }

  async createCrashReport(
    error: HttpException | Error,
    source: TelemetrySource,
  ): Promise<void> {
    try {
      if (!this.isCrashReportEnabled()) return;

      const crashReport: CrashReportAttributes = this.buildCrashReport(error);
      if (this.crashReports.includes(JSON.stringify(crashReport))) return;
      if (this.crashReports.length >= this.maxSize) {
        this.crashReports.shift();
      }
      this.crashReports.push(JSON.stringify(crashReport));
      await this.telemetryRepository.createCrashReport(crashReport, source);
    } catch (e) {}
    return;
  }

  async sendCrashReport(): Promise<void> {
    if (!this.isCrashReportEnabled()) {
      return;
    }
    const crashReport = await this.telemetryRepository.getLastCrashReport();
    if (crashReport && !crashReport.metadata.sentAt) {
      const promises = [
        this.telemetryRepository.sendTelemetryToServer(crashReport.event),
      ];
      const collectorEndpoint = process.env.CORTEX_EXPORTER_OLTP_ENDPOINT;
      if (collectorEndpoint) {
        promises.push(
          this.telemetryRepository.sendTelemetryToOTelCollector(
            collectorEndpoint,
            crashReport,
          ),
        );
      }
      await Promise.all(promises);
      await this.telemetryRepository.markLastCrashReportAsSent();
    }
    return;
  }

  async readCrashReports(
    callback: (Telemetry: Telemetry) => void,
  ): Promise<void> {
    return this.telemetryRepository.readCrashReports(callback);
  }

  private buildCrashReport(
    error: HttpException | Error,
  ): CrashReportAttributes {
    return {
      message: error.message,
      stack: error.stack,
      payload: {
        modelId: this.contextService.get('modelId') || '',
        endpoint: this.contextService.get('endpoint') || '',
        command: this.contextService.get('command') || '',
      },
      sessionId: this.contextService.get('sessionId') || '',
    };
  }

  private isCrashReportEnabled(): boolean {
    return process.env.CORTEX_CRASH_REPORT !== '0';
  }

  private isMetricsEnabled(): boolean {
    return process.env.CORTEX_METRICS !== '0';
  }

  private async catchException(): Promise<void> {
    process.on('uncaughtException', async (error: Error) => {
      await this.createCrashReport(
        error,
        this.contextService.get('source') as TelemetrySource,
      );
    });

    process.on('unhandledRejection', async (error: Error) => {
      await this.createCrashReport(
        error,
        this.contextService.get('source') as TelemetrySource,
      );
    });
  }

  async sendEvent(
    events: EventAttributes[],
    source: TelemetrySource,
  ): Promise<void> {
    try {
      if (!this.isMetricsEnabled()) return;
      const sessionId = (this.contextService.get('sessionId') as string) || '';
      const sessionEvents = events.map((event) => ({
        ...event,
        sessionId,
      }));
      await this.telemetryRepository.sendEvent(sessionEvents, source);
    } catch (e) {
      console.error('Error sending event:', e);
    }
  }

  async sendActivationEvent(source: TelemetrySource): Promise<void> {
    try {
      if (!this.isMetricsEnabled()) return;
      if (!this.lastActiveAt) {
        const currentData = await this.telemetryRepository.getAnonymizedData();
        this.lastActiveAt = currentData?.lastActiveAt;
      }
      const isActivatedToday =
        this.lastActiveAt &&
        new Date(this.lastActiveAt).getDate() === new Date().getDate();
      if (isActivatedToday) return;
      const isNewActivation = !this.lastActiveAt;
      await this.sendEvent(
        [
          {
            name: isNewActivation ? EventName.NEW_ACTIVATE : EventName.ACTIVATE,
          },
        ],
        source,
      );
      console.log('Activation event sent', isNewActivation);
      this.lastActiveAt = new Date().toISOString();
    } catch (e) {
      console.error('Error sending activation event:', e);
    }
    await this.updateAnonymousData(this.lastActiveAt);
  }

  async addEventToQueue(event: EventAttributes): Promise<void> {
    if (!this.isMetricsEnabled()) return;
    this.metricQueue.push({
      ...event,
      sessionId: this.contextService.get('sessionId') || '',
    });
    if (this.metricQueue.length >= this.maxQueueSize) {
      await this.flushMetricQueue();
    }
  }

  private async flushMetricQueue(): Promise<void> {
    if (this.metricQueue.length > 0) {
      clearInterval(this.interval);
      await this.sendEvent(this.metricQueue, TelemetrySource.CORTEX_SERVER);
      this.interval = this.flushMetricQueueInterval();
      this.metricQueue = [];
    }
  }

  private flushMetricQueueInterval(): NodeJS.Timeout {
    return setInterval(() => {
      this.flushMetricQueue();
    }, this.flushInterval);
  }

  async updateAnonymousData(
    lastActiveAt?: string | null,
  ): Promise<TelemetryAnonymized | null> {
    try {
      const currentData = await this.telemetryRepository.getAnonymizedData();
      const updatedData = {
        ...currentData,
        sessionId: currentData?.sessionId || v4(),
        ...(lastActiveAt && { lastActiveAt }),
      };
      await this.telemetryRepository.updateAnonymousData(updatedData);
      return updatedData;
    } catch (e) {
      return null;
    }
  }
}
