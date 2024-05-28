import { Injectable } from '@nestjs/common';
import { defaultCortexCppHost, defaultCortexCppPort } from 'constant';

interface ModelStat {
  id: number;
  modelId: string;
  engine?: string;
  created?: string;
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
  ): Promise<void> {
    new Promise<void>((resolve, reject) =>
      fetch(`http://${host}:${port}/inferences/server/models`).then((res) => {
        if (res.ok) {
          res
            .json()
            .then(({ data }: ModelStatResponse) => {
              if (data && Array.isArray(data) && data.length > 0) {
                console.table(
                  data.map((e) => {
                    return {
                      modelId: e.id,
                      engine: e.engine ?? 'llama.cpp', // TODO: get engine from model when it's ready
                      status: 'running',
                      created: e.created ?? new Date(),
                    };
                  }),
                );
                resolve();
              } else reject();
            })
            .catch(reject);
        } else reject();
      }),
    ).catch(() => console.log('No models running.'));
  }
}
