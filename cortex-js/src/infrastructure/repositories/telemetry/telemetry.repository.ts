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
} from '@/domain/telemetry/telemetry.interface';
import { Injectable } from '@nestjs/common';
import { join } from 'path';
import packageJson from '@/../package.json';
import axios from 'axios';
import { telemetryServerUrl } from '@/infrastructure/constants/cortex';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

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
  constructor(private readonly fileManagerService: FileManagerService) {}

  private async getTelemetryDirectory(): Promise<string> {
    const dataFolderPath = await this.fileManagerService.getDataFolderPath();
    return join(dataFolderPath, 'telemetry');
  }

  private generateChecksum(data: any): string {
    const hash = cypto.createHash('sha256');
    hash.update(JSON.stringify(data));
    return hash.digest('hex');
  }

  async sendTelemetryToServer(telemetry: Telemetry) {
    try {
      await axios.post(
        `${telemetryServerUrl}/api/v1/crash-report`,
        telemetry.event,
        {
          headers: {
            'Content-Type': 'application/json',
            'cortex-checksum': this.generateChecksum(telemetry.event),
          },
        },
      );
    } catch (error) {
      console.log(JSON.stringify(telemetry.event, null, 2));
      console.log('Error sending telemetry to server', error);
    }
  }

  async sendTelemetryToOTelCollector(endpoint: string, telemetry: Telemetry) {
    try {
      console.log('Sending telemetry to OTel collector');
      await axios.post(`${endpoint}/v1/logs`, telemetry.event, {
        headers: {
          'Content-Type': 'application/json',
        },
      });
    } catch (error) {
      console.log('Error sending telemetry to OTel collector', error);
    }
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
    } catch (error) {
      console.log('Error marking crash report as sent', error);
    }
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
      const { data } = await this.fileManagerService.getLastLine(
        join(await this.getTelemetryDirectory(), this.crashReportFileName),
      );
      if (!data) {
        return null;
      }
      return JSON.parse(data) as Telemetry;
    } catch (error) {
      console.log('Error reading crash report', error);
      return null;
    }
  }

  async createCrashReport(
    crashReport: CrashReportAttributes,
    source: TelemetrySource,
  ): Promise<void> {
    const telemetryEvent = await this.convertCrashReportToTelemetryEvent(
      crashReport,
      source,
    );
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

  private async convertCrashReportToTelemetryEvent(
    crashReport: CrashReportAttributes,
    // move this to telemetryResource
    source: TelemetrySource,
  ): Promise<TelemetryEvent> {
    const gpus = (await systemInformation.graphics()).controllers.map(
      ({ model, vendor, vram, vramDynamic }) => ({
        model,
        vendor,
        vram,
        vramDynamic,
      }),
    );
    const resourceAttributes: Attribute[] = Object.entries({
      ...this.telemetryResource,
      'service.name': 'crash-reporting',
      cpu: os.cpus()[0].model,
      gpus: JSON.stringify(gpus),
      source,
    }).map(([key, value]) => ({
      key,
      value: { stringValue: value },
    }));
    const telemetryLogAttributes: Attribute[] = Object.entries(crashReport).map(
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
              startTimeUnixNano: (
                BigInt(Date.now()) * BigInt(1000000)
              ).toString(),
              body: { stringValue: crashReport.message },
              severityText: 'ERROR',
              attributes: telemetryLogAttributes,
            },
          ],
        },
      ],
    };
  }
}
