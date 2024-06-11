import { ChatCompletionMessageParam } from 'openai/resources';

export interface BenchmarkConfig {
  api: {
    base_url: string;
    api_key: string;
    parameters: {
      messages: ChatCompletionMessageParam[];
      model: string;
      stream?: boolean;
      max_tokens?: number;
      stop?: string[];
      frequency_penalty?: number;
      presence_penalty?: number;
      temperature?: number;
      top_p?: number;
    };
  };
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
