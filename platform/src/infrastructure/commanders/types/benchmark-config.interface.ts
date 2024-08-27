import { Cortex } from '@cortexso/cortex.js';

export interface ApiConfig {
  base_url: string;
  api_key: string;
  parameters: ParametersConfig;
}

export interface ParametersConfig {
  messages: Cortex.ChatCompletionMessageParam[];
  model: string;
  stream?: boolean;
  max_tokens?: number;
  stop?: string[];
  frequency_penalty?: number;
  presence_penalty?: number;
  temperature?: number;
  top_p?: number;
}

export interface BenchmarkConfig {
  api: ApiConfig;
  prompts?: {
    min: number;
    max: number;
    samples: number;
  };
  output: string;
  concurrency: number;
  num_rounds: number;
  hardware: string[];
}
