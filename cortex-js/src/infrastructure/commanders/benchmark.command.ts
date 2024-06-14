import { CommandRunner, SubCommand, Option } from 'nest-commander';
import { BenchmarkCliUsecases } from './usecases/benchmark.cli.usecases';
import { BenchmarkConfig } from './types/benchmark-config.interface';

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

  async run(
    _input: string[],
    options?: Partial<BenchmarkConfig>,
  ): Promise<void> {
    return this.benchmarkUsecases.benchmark(options ?? {});
  }

  @Option({
    flags: '-n, --num_rounds <num_rounds>',
    description: 'Number of rounds to run the benchmark',
  })
  parseRounds(value: number) {
    return value;
  }

  @Option({
    flags: '-c, --concurrency <concurrency>',
    description: 'Number of concurrent requests to run the benchmark',
  })
  parseConcurrency(value: number) {
    return value;
  }

  @Option({
    flags: '-o, --output <output>',
    description: 'Output format for the benchmark results. json or table',
  })
  parseOutput(value: string) {
    return value;
  }
}
