import { Injectable } from '@nestjs/common';
import si from 'systeminformation';
import fs, { existsSync, mkdirSync, readFileSync, writeFileSync } from 'fs';
import OpenAI from 'openai';
import { Presets, SingleBar } from 'cli-progress';
import yaml from 'js-yaml';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { join } from 'path';
import { ModelsCliUsecases } from './models.cli.usecases';
import { spawn } from 'child_process';
import { BenchmarkConfig } from '@commanders/types/benchmark-config.interface';
import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';
import { inspect } from 'util';
import { defaultBenchmarkConfiguration } from '@/infrastructure/constants/benchmark';

@Injectable()
export class BenchmarkCliUsecases {
  constructor(
    private readonly modelsCliUsecases: ModelsCliUsecases,
    private readonly cortexUsecases: CortexUsecases,
    private readonly fileService: FileManagerService,
  ) {}

  config: BenchmarkConfig;
  openai?: OpenAI;
  /**
   * Benchmark and analyze the performance of a specific AI model using a variety of system resources
   */
  async benchmark() {
    return this.getBenchmarkConfig().then((config) => {
      this.config = config;

      // TODO: Using OpenAI client or Cortex client to benchmark?
      this.openai = new OpenAI({
        apiKey: this.config.api.api_key,
        baseURL: this.config.api.base_url,
        timeout: 20 * 1000,
      });

      spawn('cortex', ['serve'], {
        detached: false,
      });

      return this.cortexUsecases
        .startCortex()
        .then(() =>
          this.modelsCliUsecases.startModel(this.config.api.parameters.model),
        )
        .then(() => this.runBenchmarks())
        .then(() => process.exit(0));
    });
  }

  /**
   * Get the benchmark configuration
   * @returns the benchmark configuration
   */
  private async getBenchmarkConfig() {
    const benchmarkFolder = await this.fileService.getBenchmarkPath();
    const configurationPath = join(benchmarkFolder, 'config.yaml');
    if (existsSync(configurationPath)) {
      return yaml.load(
        readFileSync(configurationPath, 'utf8'),
      ) as BenchmarkConfig;
    } else {
      const config = yaml.dump(defaultBenchmarkConfiguration);
      if (!existsSync(benchmarkFolder)) {
        mkdirSync(benchmarkFolder, {
          recursive: true,
        });
      }
      await writeFileSync(configurationPath, config, 'utf8');
      return defaultBenchmarkConfiguration;
    }
  }

  /**
   * Get the system resources for benchmarking
   * using the systeminformation library
   * @returns the system resources
   */
  private async getSystemResources() {
    return {
      cpu: await si.currentLoad(),
      mem: await si.mem(),
      gpu: (await si.graphics()).controllers,
    };
  }

  /**
   * Get the resource change between two data points
   * @param startData the start data point
   * @param endData the end data point
   * @returns the resource change
   */
  private async getResourceChange(startData: any, endData: any) {
    return {
      cpu:
        startData.cpu && endData.cpu
          ? ((endData.cpu.currentload - startData.cpu.currentload) /
              startData.cpu.currentload) *
            100
          : null,
      mem:
        startData.mem && endData.mem
          ? ((endData.mem.used - startData.mem.used) / startData.mem.total) *
            100
          : null,
    };
  }

  /**
   * Benchmark a user using the OpenAI API
   * @returns
   */
  private async benchmarkUser() {
    const startResources = await this.getSystemResources();
    const start = Date.now();
    let tokenCount = 0;
    let firstTokenTime = null;

    try {
      const stream = await this.openai!.chat.completions.create({
        model: this.config.api.parameters.model,
        messages: this.config.api.parameters.messages,
        max_tokens: this.config.api.parameters.max_tokens,
        stream: true,
      });

      for await (const chunk of stream) {
        if (!firstTokenTime && chunk.choices[0]?.delta?.content) {
          firstTokenTime = Date.now();
        }
        tokenCount += (chunk.choices[0]?.delta?.content || '').split(
          /\s+/,
        ).length;
      }
    } catch (error) {
      console.error('Error during API call:', error);
      return null;
    }

    const latency = Date.now() - start;
    const ttft = firstTokenTime ? firstTokenTime - start : null;
    const endResources = await this.getSystemResources();
    const resourceChange = await this.getResourceChange(
      startResources,
      endResources,
    );

    return {
      tokens: this.config.api.parameters.max_tokens,
      token_length: tokenCount, // Dynamically calculated token count
      latency,
      resourceChange,
      tpot: tokenCount ? latency / tokenCount : 0,
      throughput: tokenCount / (latency / 1000),
      ttft,
    };
  }

  /**
   * Calculate the percentiles of the data
   * @param data the data to calculate percentiles for
   * @param percentile the percentile to calculate
   * @returns the percentile value
   */
  private calculatePercentiles(data: number[], percentile: number) {
    if (data.length === 0) return null;
    const sorted = data
      .filter((x: number) => x !== null)
      .sort((a: number, b: number) => a - b);
    const pos = (percentile / 100) * sorted.length;
    if (pos < 1) return sorted[0];
    if (pos >= sorted.length) return sorted[sorted.length - 1];
    const lower = sorted[Math.floor(pos) - 1];
    const upper = sorted[Math.ceil(pos) - 1];
    return lower + (upper - lower) * (pos - Math.floor(pos));
  }

  /**
   * Run the benchmarks
   */
  private async runBenchmarks() {
    const allResults: any[] = [];
    const rounds = this.config.num_rounds || 1;

    const bar = new SingleBar({}, Presets.shades_classic);
    bar.start(rounds, 0);

    for (let i = 0; i < rounds; i++) {
      const roundResults = [];
      const hardwareBefore = await this.getSystemResources();

      for (let j = 0; j < this.config.concurrency; j++) {
        const result = await this.benchmarkUser();
        if (result) {
          roundResults.push(result);
        }
      }

      const hardwareAfter = await this.getSystemResources();
      const hardwareChanges = await this.getResourceChange(
        hardwareBefore,
        hardwareAfter,
      );

      allResults.push({
        round: i + 1,
        results: roundResults,
        hardwareChanges,
      });

      bar.update(i + 1);
    }

    const metrics: any = {
      p50: {},
      p75: {},
      p95: {},
    };
    const keys = ['latency', 'tpot', 'throughput', 'ttft'];
    keys.forEach((key) => {
      const data = allResults.flatMap((r) =>
        r.results.map((res: object) => res[key as keyof typeof res]),
      );
      metrics.p50[key] = this.calculatePercentiles(data, 50);
      metrics.p75[key] = this.calculatePercentiles(data, 75);
      metrics.p95[key] = this.calculatePercentiles(data, 95);
    });

    const output = {
      hardware: await this.getSystemResources(),
      results: allResults,
      metrics,
    };
    bar.stop();

    const outputFilePath = join(
      await this.fileService.getBenchmarkPath(),
      'output.json',
    );
    fs.writeFileSync(outputFilePath, JSON.stringify(output, null, 2));
    console.log(`Benchmark results and metrics saved to ${outputFilePath}`);

    console.log(
      inspect(output, { showHidden: false, depth: null, colors: true }),
    );
  }
}
