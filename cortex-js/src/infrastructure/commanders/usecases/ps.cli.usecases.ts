import { Injectable } from '@nestjs/common';
import { defaultCortexCppHost, defaultCortexCppPort } from 'constant';

interface ModelStat {
  id: number;
  modelId: string;
  engine?: string;
  start_time?: string;
  status: string;
  vram?: string;
  ram?: string;
}
interface ModelStatResponse {
  object: string;
  data: ModelStat[];
}
@Injectable()
export class PSCliUsecases {
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
      fetch(`http://${host}:${port}/inferences/server/models`)
        .then((res) => {
          if (res.ok) {
            res
              .json()
              .then(({ data }: ModelStatResponse) => {
                if (data && Array.isArray(data) && data.length > 0) {
                  resolve(data);
                } else reject();
              })
              .catch(reject);
          } else reject();
        })
        .catch(reject),
    ).catch(() => []);
  }
}
