import { CommandRunner, SubCommand } from 'nest-commander';
import { BenchmarkCliUsecases } from './usecases/benchmark.cli.usecases';

@SubCommand({
  name: 'benchmark',
  subCommands: [],
  description:
    'Benchmark and analyze the performance of a specific AI model using a variety of system resources',
})
export class BenchmarkCommand extends CommandRunner {
  constructor(private readonly benchmarkUsecases: BenchmarkCliUsecases) {
    super();
  }

  async run(): Promise<void> {
    return this.benchmarkUsecases.benchmark();
  }
}
