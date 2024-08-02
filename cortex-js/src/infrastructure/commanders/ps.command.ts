import ora from 'ora';
import systeminformation from 'systeminformation';
import { SubCommand } from 'nest-commander';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';
import { ModelStat } from './types/model-stat.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { BaseCommand } from './base.command';
import { HttpStatus } from '@nestjs/common';
import { Engines } from './types/engine.interface';
import { ModelStatResponse } from '../providers/cortex/cortex.provider';
import { firstValueFrom } from 'rxjs';
import { HttpService } from '@nestjs/axios';
import { FileManagerService } from '../services/file-manager/file-manager.service';
import { CORTEX_CPP_MODELS_URL } from '../constants/cortex';

@SubCommand({
  name: 'ps',
  description: 'Show running models and their status',
})
@SetCommandContext()
export class PSCommand extends BaseCommand {
  constructor(
    readonly cortexUsecases: CortexUsecases,
    private readonly httpService: HttpService,
    private readonly fileService: FileManagerService,
  ) {
    super(cortexUsecases, fileService);
  }
  async runCommand(): Promise<void> {
    const runningSpinner = ora('Running PS command...').start();
    return this.getModels()
      .then((models: ModelStat[]) => {
        runningSpinner.succeed();
        if (models.length) console.table(models);
        else console.log('No models running');
      })
      .then(async () => {
        const cpuUsage = (
          await systeminformation.currentLoad()
        ).currentLoad.toFixed(2);
        const gpusLoad = [];
        const gpus = await systeminformation.graphics();
        for (const gpu of gpus.controllers) {
          const totalVram = gpu.vram || 0;
          gpusLoad.push({
            totalVram,
          });
        }
        const memoryData = await systeminformation.mem();
        const memoryUsage = (
          (memoryData.active / memoryData.total) *
          100
        ).toFixed(2);
        const consumedTable = {
          'CPU Usage': `${cpuUsage}%`,
          'Memory Usage': `${memoryUsage}%`,
        } as {
          'CPU Usage': string;
          'Memory Usage': string;
          VRAM?: string;
        };

        if (
          gpusLoad.length > 0 &&
          gpusLoad.filter((gpu) => gpu.totalVram > 0).length > 0
        ) {
          consumedTable['VRAM'] = gpusLoad
            .map((gpu) => `${gpu.totalVram} MB`)
            .join(', ');
        }
        console.table([consumedTable]);
      });
  }

  /**
   * Get models running in the Cortex C++ server
   */
  async getModels(): Promise<ModelStat[]> {
    const configs = await this.fileService.getConfig();
    const runningSpinner = ora('Getting models...').start();
    return new Promise<ModelStat[]>((resolve, reject) =>
      firstValueFrom(
        this.httpService.get(
          CORTEX_CPP_MODELS_URL(configs.cortexCppHost, configs.cortexCppPort),
        ),
      )
        .then((res) => {
          const data = res.data as ModelStatResponse;
          if (
            res.status === HttpStatus.OK &&
            data &&
            Array.isArray(data.data) &&
            data.data.length > 0
          ) {
            runningSpinner.succeed();
            resolve(
              data.data.map((e) => {
                const startTime = e.start_time ?? new Date();
                const currentTime = new Date();
                const duration =
                  currentTime.getTime() - new Date(startTime).getTime();
                return {
                  modelId: e.id,
                  engine: e.engine ?? Engines.llamaCPP,
                  status: 'running',
                  duration: this.formatDuration(duration),
                  ram: e.ram ?? '-',
                  vram: e.vram ?? '-',
                };
              }),
            );
          } else reject();
        })
        .catch(reject),
    ).catch(() => {
      runningSpinner.succeed('');
      return [];
    });
  }

  private formatDuration(milliseconds: number): string {
    const days = Math.floor(milliseconds / (1000 * 60 * 60 * 24));
    const hours = Math.floor(
      (milliseconds % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60),
    );
    const minutes = Math.floor((milliseconds % (1000 * 60 * 60)) / (1000 * 60));
    const seconds = Math.floor((milliseconds % (1000 * 60)) / 1000);

    let formattedDuration = '';

    if (days > 0) {
      formattedDuration += `${days}d `;
    }
    if (hours > 0) {
      formattedDuration += `${hours}h `;
    }
    if (minutes > 0) {
      formattedDuration += `${minutes}m `;
    }
    if (seconds > 0) {
      formattedDuration += `${seconds}s `;
    }

    return formattedDuration.trim();
  }
}
