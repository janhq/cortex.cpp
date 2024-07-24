import { HttpStatus, Injectable } from '@nestjs/common';
import ora from 'ora';
import { CORTEX_CPP_MODELS_URL } from '@/infrastructure/constants/cortex';
import { HttpService } from '@nestjs/axios';
import { firstValueFrom } from 'rxjs';
import { ModelStat } from '@commanders/types/model-stat.interface';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { Engines } from '../types/engine.interface';

interface ModelStatResponse {
  object: string;
  data: any;
}
@Injectable()
export class PSCliUsecases {
  constructor(
    private readonly httpService: HttpService,
    private readonly fileService: FileManagerService,
  ) {}
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
