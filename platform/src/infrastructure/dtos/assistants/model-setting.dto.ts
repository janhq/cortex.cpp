import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsOptional } from 'class-validator';

export class ModelSettingDto {
  @ApiProperty({
    type: 'number',
    minimum: 0,
    maximum: 1,
    required: false,
    default: 1,
    description: `What sampling temperature to use, between 0 and 2. Higher values like 0.8 will make the output more random, while lower values like 0.2 will make it more focused and deterministic.`,
  })
  temperature: number;

  @ApiProperty({
    type: 'number',
    minimum: 0,
    maximum: 1,
    required: false,
    default: 1,
    description: `An alternative to sampling with temperature, called nucleus sampling, where the model considers the results of the tokens with top_p probability mass. So 0.1 means only the tokens comprising the top 10% probability mass are considered.\nWe generally recommend altering this or temperature but not both.`,
  })
  top_p: number;

  @ApiProperty({
    required: false,
    example: '',
    description: 'GGUF metadata: tokenizer.chat_template',
  })
  prompt_template?: string;

  @ApiProperty({
    required: false,
    example: [],
    description:
      'Defines specific tokens or phrases at which the model will stop generating further output.',
    default: [],
  })
  @IsArray()
  @IsOptional()
  stop?: string[];

  @ApiProperty({
    required: false,
    type: 'number',
    example: 0,
    description:
      'Adjusts the likelihood of the model repeating words or phrases in its output.',
  })
  frequency_penalty?: number;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 0,
    description:
      'Influences the generation of new and varied concepts in the model’s output.',
  })
  presence_penalty?: number;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 4096,
    default: 4096,
    description:
      'The context length for model operations varies; the maximum depends on the specific model used.',
  })
  ctx_len?: number;

  @ApiProperty({
    required: false,
    type: 'boolean',
    example: true,
    default: true,
    description: 'Enable real-time data processing for faster predictions.',
  })
  stream?: boolean;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 2048,
    default: 2048,
    description:
      'The maximum number of tokens the model will generate in a single response.',
  })
  max_tokens?: number;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 1,
    default: 1,
    description: 'The number of layers to load onto the GPU for acceleration.',
  })
  ngl?: number;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 1,
    default: 1,
    description: 'Number of parallel sequences to decode',
  })
  n_parallel?: number;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 1,
    default: 1,
    description:
      'Determines CPU inference threads, limited by hardware and OS. (Maximum determined by system)',
  })
  cpu_threads?: number;

  @ApiProperty({
    required: false,
    type: 'string',
    example: '',
    default: '',
    description: 'The prompt to use for internal configuration',
  })
  pre_prompt?: string;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 0,
    default: 0,
    description: 'The batch size for prompt eval step',
  })
  n_batch?: number;

  @ApiProperty({
    required: false,
    type: 'boolean',
    example: true,
    default: true,
    description: 'To enable prompt caching or not',
  })
  caching_enabled?: boolean;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 0,
    default: 0,
    description: 'Group attention factor in self-extend',
  })
  grp_attn_n?: number;

  @ApiProperty({
    required: false,
    type: 'number',
    example: 0,
    default: 0,
    description: 'Group attention width in self-extend',
  })
  grp_attn_w?: number;

  @ApiProperty({
    required: false,
    type: 'boolean',
    example: false,
    default: false,
    description: 'Prevent system swapping of the model to disk in macOS',
  })
  mlock?: boolean;

  @ApiProperty({
    required: false,
    type: 'string',
    example: '',
    default: '',
    description:
      'You can constrain the sampling using GBNF grammars by providing path to a grammar file',
  })
  grammar_file?: string;

  @ApiProperty({
    required: false,
    type: 'boolean',
    example: true,
    default: true,
    description: 'To enable Flash Attention, default is true',
  })
  flash_attn?: boolean;

  @ApiProperty({
    required: false,
    type: 'string',
    example: '',
    default: '',
    description: 'KV cache type: f16, q8_0, q4_0, default is f16',
  })
  cache_type?: string;

  @ApiProperty({
    required: false,
    type: 'boolean',
    example: true,
    default: true,
    description: 'To enable mmap, default is true',
  })
  use_mmap?: boolean;
}
