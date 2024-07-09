import ora from 'ora';
import systeminformation from 'systeminformation';
import { CommandRunner, SubCommand } from 'nest-commander';
import { PSCliUsecases } from './usecases/ps.cli.usecases';
import { SetCommandContext } from './decorators/CommandContext';
import { ContextService } from '../services/context/context.service';
import { ModelStat } from './types/model-stat.interface';

@SubCommand({
  name: 'ps',
  description: 'Show running models and their status',
})
@SetCommandContext()
export class PSCommand extends CommandRunner {
  constructor(
    private readonly usecases: PSCliUsecases,
    readonly contextService: ContextService,
  ) {
    super();
  }
  async run(): Promise<void> {
    const runningSpinner = ora('Running PS command...').start();
    let checkingSpinner: ora.Ora
    return this.usecases
      .getModels()
      .then((models: ModelStat[]) => {
        runningSpinner.succeed();
        console.table(models);
      })
      .then(() => {
        checkingSpinner = ora('Checking API server...').start();
        return this.usecases.isAPIServerOnline();
      })
      .then((isOnline) => {
        checkingSpinner.succeed(isOnline ? 'API server is online' : 'API server is offline');
      })
      .then(async () => {
        const table = [];
        const cpuUsage = (await systeminformation.currentLoad()).currentLoad.toFixed(2);
        const gpusLoad = [];
        const gpus = await systeminformation.graphics();
        for (const gpu of gpus.controllers) {
          const totalVram = gpu.vram || 0;
          gpusLoad.push({
            totalVram,
          });
        }
        const memoryData = await systeminformation.mem();
        const memoryUsage =  (memoryData.active / memoryData.total * 100).toFixed(2)
        table.push({
          'CPU Usage': `${cpuUsage}%`,
          'Memory Usage': `${memoryUsage}%`,
          'VRAM': gpusLoad.map((gpu) => `${gpu.totalVram} MB`).join('\n'),
        });
        console.table(table);
      })
  }
}