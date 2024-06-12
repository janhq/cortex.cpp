import { BenchmarkConfig } from '../commanders/types/benchmark-config.interface';

export const defaultBenchmarkConfiguration: BenchmarkConfig = {
  api: {
    base_url: 'http://127.0.0.1:1337/',
    api_key: '<api_key>',
    parameters: {
      messages: [
        {
          content: 'You are a helpful assistant.',
          role: 'system',
        },
        {
          content: 'Hello!',
          role: 'user',
        },
      ],
      model: 'tinyllama',
      stream: true,
      max_tokens: 2048,
      stop: [],
      frequency_penalty: 0,
      presence_penalty: 0,
      temperature: 0.7,
      top_p: 0.95,
    },
  },
  prompts: {
    min: 102,
    max: 2048,
    samples: 10,
  },
  output: 'json',
  hardware: ['cpu', 'gpu', 'psu', 'chassis', 'ram'],
  concurrency: 1,
  num_rounds: 10,
};
