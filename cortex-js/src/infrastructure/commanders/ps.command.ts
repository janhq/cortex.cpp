import ora from 'ora';
import systeminformation from 'systeminformation';
import { SubCommand } from 'nest-commander';
import { PSCliUsecases } from './usecases/ps.cli.usecases';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';
import { ModelStat } from './types/model-stat.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { UseInterceptors } from '@nestjs/common';
import { ServerHealthCheckInterceptor } from '../interceptors/server-health-check.interceptor';
import { BaseCommand } from './base.command';

@SubCommand({
  name: 'ps',
  description: 'Show running models and their status',
})
@SetCommandContext()
@UseInterceptors(ServerHealthCheckInterceptor)
export class PSCommand extends BaseCommand {
  constructor(
    private readonly usecases: PSCliUsecases,
    private readonly contextService: ContextService,
    private readonly cortexUsecases: CortexUsecases,
  ) {
    super(cortexUsecases);
  }
  async runCommand(): Promise<void> {
    const runningSpinner = ora('Running PS command...').start();
    let checkingSpinner: ora.Ora;
    return this.usecases
      .getModels()
      .then((models: ModelStat[]) => {
        runningSpinner.succeed();
        console.table(models);
      })
      .then(() => {
        checkingSpinner = ora('Checking API server...').start();
        return this.cortexUsecases.isAPIServerOnline();
      })
      .then((isOnline) => {
        checkingSpinner.succeed(
          isOnline ? 'API server is online' : 'API server is offline',
        );
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
}
