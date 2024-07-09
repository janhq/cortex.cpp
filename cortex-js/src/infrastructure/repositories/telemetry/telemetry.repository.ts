import * as cypto from 'crypto';
import os from 'os';
import systemInformation from 'systeminformation';
import { TelemetryRepository } from '@/domain/repositories/telemetry.interface';
import {
  Attribute,
  CrashReportAttributes,
  TelemetrySource,
  TelemetryEvent,
  TelemetryResource,
  Telemetry,
  TelemetryEventMetadata,
  EventAttributes,
  TelemetryAnonymized,
  BenchmarkHardware,
} from '@/domain/telemetry/telemetry.interface';
import { Injectable } from '@nestjs/common';
import { join } from 'path';
import packageJson from '@/../package.json';
import axios from 'axios';
import { telemetryServerUrl } from '@/infrastructure/constants/cortex';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { ModelStat } from '@/infrastructure/commanders/types/model-stat.interface';

// refactor using convert to dto
@Injectable()
export class TelemetryRepositoryImpl implements TelemetryRepository {
  private readonly telemetryResource: TelemetryResource = {
    osName: process.platform,
    osVersion: process.version,
    architecture: process.arch,
    appVersion: packageJson.version,
  };

  private readonly crashReportFileName = 'crash-report.jsonl';
  private readonly anonymizedDataFileName = 'session.json';
  constructor(private readonly fileManagerService: FileManagerService) {}

  private async getTelemetryDirectory(): Promise<string> {
    const dataFolderPath = await this.fileManagerService.getDataFolderPath();
    await this.fileManagerService.createFolderIfNotExistInDataFolder(
      'telemetry',
    );
    return join(dataFolderPath, 'telemetry');
  }

  private generateChecksum(data: any): string {
    const hash = cypto.createHash('sha256');
    hash.update(JSON.stringify(data));
    return hash.digest('hex');
  }

  async sendTelemetryToServer(
    telemetryEvent: Telemetry['event'],
    type: 'crash-report' | 'metrics' = 'crash-report',
  ): Promise<void> {
    try {
      await axios.post(`${telemetryServerUrl}/api/v1/${type}`, telemetryEvent, {
        headers: {
          'Content-Type': 'application/json',
          'cortex-checksum': this.generateChecksum(telemetryEvent),
          timeout: 1000,
        },
      });
    } catch (error) {}
  }

  async sendBenchmarkToServer(data: {
    hardware: BenchmarkHardware;
    results: any;
    metrics: any;
    model: ModelStat;
    sessionId: string;
  }): Promise<void> {
    try {
      await axios.post(`${telemetryServerUrl}/api/v1/benchmark`, data, {
        headers: {
          'Content-Type': 'application/json',
          'cortex-checksum': this.generateChecksum(data),
        },
      });
    } catch (error) {}
  }

  async sendTelemetryToOTelCollector(endpoint: string, telemetry: Telemetry) {
    try {
      console.log('Sending telemetry to OTel collector');
      await axios.post(`${endpoint}/v1/logs`, telemetry.event, {
        headers: {
          'Content-Type': 'application/json',
        },
      });
    } catch (error) {}
  }

  async markLastCrashReportAsSent(): Promise<void> {
    try {
      const { data, position } = await this.fileManagerService.getLastLine(
        join(await this.getTelemetryDirectory(), this.crashReportFileName),
      );
      const Telemetry = JSON.parse(data) as Telemetry;
      Telemetry.metadata.sentAt = new Date().toISOString();
      await this.fileManagerService.modifyLine(
        join(await this.getTelemetryDirectory(), this.crashReportFileName),
        JSON.stringify(Telemetry),
        position,
      );
    } catch (error) {}
  }

  async readCrashReports(callback: (Telemetry: Telemetry) => void) {
    this.fileManagerService.readLines(
      join(await this.getTelemetryDirectory(), this.crashReportFileName),
      (line: string) => {
        const data = JSON.parse(line) as Telemetry;
        callback(data);
      },
    );
  }

  async getLastCrashReport(): Promise<Telemetry | null> {
    try {
      await this.fileManagerService.createFolderIfNotExistInDataFolder(
        'telemetry',
      );
      const { data } = await this.fileManagerService.getLastLine(
        join(await this.getTelemetryDirectory(), this.crashReportFileName),
      );
      if (!data) {
        return null;
      }
      return JSON.parse(data) as Telemetry;
    } catch (error) {
      return null;
    }
  }

  async createCrashReport(
    crashReport: CrashReportAttributes,
    source: TelemetrySource,
  ): Promise<void> {
    const telemetryEvent = await this.convertToTelemetryEvent({
      attributes: crashReport,
      source,
      type: 'crash-report',
    });
    const metadata: TelemetryEventMetadata = {
      createdAt: new Date().toISOString(),
      sentAt: null,
    };
    return this.fileManagerService.append(
      join(await this.getTelemetryDirectory(), this.crashReportFileName),
      JSON.stringify({
        metadata,
        event: {
          resourceLogs: [telemetryEvent],
        },
      } as Telemetry),
    );
  }

  private async convertToTelemetryEvent({
    attributes,
    source,
    type,
  }: {
    attributes: CrashReportAttributes | EventAttributes;
    source: TelemetrySource;
    type: 'crash-report' | 'metrics';
  }): Promise<TelemetryEvent> {
    const gpus = (await systemInformation.graphics()).controllers.map(
      ({ model, vendor, vram, vramDynamic }) => ({
        model,
        vendor,
        vram,
        vramDynamic,
      }),
    );

    const body =
      type === 'crash-report'
        ? (attributes as CrashReportAttributes).message
        : (attributes as EventAttributes).name;

    const severity = type === 'crash-report' ? 'ERROR' : 'INFO';

    const resourceAttributes: Attribute[] = Object.entries({
      ...this.telemetryResource,
      'service.name': type,
      cpu: os.cpus()[0].model,
      gpus: JSON.stringify(gpus),
      source,
    }).map(([key, value]) => ({
      key,
      value: { stringValue: value },
    }));
    const telemetryLogAttributes: Attribute[] = Object.entries(attributes).map(
      ([key, value]) => {
        if (typeof value === 'object') {
          return {
            key,
            value: {
              kvlist_value: {
                values: Object.entries(value).map(([k, v]) => ({
                  key: k,
                  value: { stringValue: v as string },
                })),
              },
            },
          };
        }
        return {
          key,
          value: { stringValue: value },
        };
      },
    );

    return {
      resource: {
        attributes: resourceAttributes,
      },
      scopeLogs: [
        {
          scope: {},
          logRecords: [
            {
              traceId: cypto.randomBytes(16).toString('hex'),
              timeUnixNano: (
                BigInt(Date.now()) * BigInt(1000000)
              ).toString(),
              body: { stringValue: body },
              severityText: severity,
              attributes: telemetryLogAttributes,
            },
          ],
        },
      ],
    };
  }

  async sendEvent(
    events: EventAttributes[],
    source: TelemetrySource,
  ): Promise<void> {
    const telemetryEvents = await Promise.all(
      events.map(async (event) =>
        this.convertToTelemetryEvent({
          attributes: event,
          source,
          type: 'metrics',
        }),
      ),
    );
    await this.sendTelemetryToServer(
      {
        resourceLogs: telemetryEvents,
      },
      'metrics',
    );
  }

  async getAnonymizedData(): Promise<TelemetryAnonymized | null> {
    const content = await this.fileManagerService.readFile(
      join(await this.getTelemetryDirectory(), this.anonymizedDataFileName),
    );

    if (!content) {
      return null;
    }

    const data = JSON.parse(content) as TelemetryAnonymized;
    return data;
  }

  async updateAnonymousData(data: TelemetryAnonymized): Promise<void> {
    return this.fileManagerService.writeFile(
      join(await this.getTelemetryDirectory(), this.anonymizedDataFileName),
      JSON.stringify(data),
    );
  }
}
