import { HttpStatus, Injectable } from '@nestjs/common';
import {
  CORTEX_CPP_MODELS_URL,
  defaultCortexCppHost,
  defaultCortexCppPort,
} from '@/infrastructure/constants/cortex';
import { HttpService } from '@nestjs/axios';
import { firstValueFrom } from 'rxjs';

export interface ModelStat {
  modelId: string;
  engine?: string;
  duration?: string;
  status: string;
  vram?: string;
  ram?: string;
}
interface ModelStatResponse {
  object: string;
  data: any;
}
@Injectable()
export class PSCliUsecases {
  constructor(private readonly httpService: HttpService) {}
  /**
   * Get models running in the Cortex C++ server
   * @param host Cortex host address
   * @param port Cortex port address
   */
  async getModels(
    host: string = defaultCortexCppHost,
    port: number = defaultCortexCppPort,
  ): Promise<ModelStat[]> {
    return new Promise<ModelStat[]>((resolve, reject) =>
      firstValueFrom(this.httpService.get(CORTEX_CPP_MODELS_URL(host, port)))
        .then((res) => {
          const data = res.data as ModelStatResponse;
          if (
            res.status === HttpStatus.OK &&
            data &&
            Array.isArray(data.data) &&
            data.data.length > 0
          ) {
            resolve(
              data.data.map((e) => {
                const startTime = e.start_time ?? new Date();
                const currentTime = new Date();
                const duration =
                  currentTime.getTime() - new Date(startTime).getTime();
                return {
                  modelId: e.id,
                  engine: e.engine ?? 'cortex.llamacpp',
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
    ).catch(() => []);
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
