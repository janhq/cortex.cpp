import { Model } from '@/domain/models/model.interface';
import { ApiProperty } from '@nestjs/swagger';
import {
  IsArray,
  IsBoolean,
  IsNumber,
  IsOptional,
  IsString,
} from 'class-validator';

export class ModelDto implements Partial<Model> {
  @ApiProperty({
    example: 'mistral',
    description:
      'The model identifier, which can be referenced in the API endpoints.',
  })
  @IsOptional()
  id: string;

  // Prompt Settings
  @ApiProperty({
    example: `You are an expert in {subject}. Provide a detailed and thorough explanation on the topic of {topic}.`,
    description:
      "A predefined text or framework that guides the AI model's response generation.",
  })
  @IsOptional()
  prompt_template?: string;

  @ApiProperty({
    type: [String],
    example: ['End'],
    description:
      'Defines specific tokens or phrases that signal the model to stop producing further output.',
  })
  @IsArray()
  @IsOptional()
  stop?: string[];

  // Results Preferences

  @ApiProperty({
    example: 4096,
    description:
      'Sets the upper limit on the number of tokens the model can generate in a single output.',
  })
  @IsOptional()
  @IsNumber()
  max_tokens?: number;

  @ApiProperty({
    example: 0.7,
    description: "Influences the randomness of the model's output.",
  })
  @IsOptional()
  @IsNumber()
  temperature?: number;

  @ApiProperty({
    example: 0.95,
    description: 'Sets probability threshold for more relevant outputs',
  })
  @IsOptional()
  @IsNumber()
  top_p?: number;

  @ApiProperty({
    example: true,
    description:
      'Determines the format for output generation. If set to `true`, the output is generated continuously, allowing for real-time streaming of responses. If set to `false`, the output is delivered in a single JSON file.',
  })
  @IsOptional()
  @IsBoolean()
  stream?: boolean;

  @ApiProperty({
    example: 0,
    description:
      'Modifies the likelihood of the model repeating the same words or phrases within a single output.',
  })
  @IsOptional()
  @IsNumber()
  frequency_penalty?: number;

  @ApiProperty({
    example: 0,
    description:
      'Reduces the likelihood of repeating tokens, promoting novelty in the output.',
  })
  @IsOptional()
  @IsNumber()
  presence_penalty?: number;

  // Engine Settings
  @ApiProperty({ description: 'Determines GPU layer usage.', example: 32 })
  @IsOptional()
  @IsNumber()
  ngl?: number;

  @ApiProperty({
    description:
      'The context length for model operations varies; the maximum depends on the specific model used.',
    example: 4096,
  })
  @IsOptional()
  @IsNumber()
  ctx_len?: number;

  @ApiProperty({
    description:
      'Determines CPU inference threads, limited by hardware and OS.',
    example: 10,
  })
  @IsOptional()
  @IsNumber()
  cpu_threads?: number;

  @ApiProperty({
    description: 'The prompt to use for internal configuration',
    example:
      'You are an assistant with expert knowledge in {subject}. Please provide a detailed and accurate response to the following query: {query}. Ensure that your response is clear, concise, and informative.',
  })
  @IsOptional()
  @IsString()
  pre_prompt?: string;

  @ApiProperty({
    description: 'The batch size for prompt eval step',
    example: 512,
  })
  @IsOptional()
  @IsNumber()
  n_batch?: number;

  @ApiProperty({
    description: 'To enable prompt caching or not',
    example: true,
  })
  @IsOptional()
  @IsBoolean()
  caching_enabled?: boolean;

  @ApiProperty({
    description: 'Group attention factor in self-extend',
    example: 1,
  })
  @IsOptional()
  @IsNumber()
  grp_attn_n?: number;

  @ApiProperty({
    description: 'Group attention width in self-extend',
    example: 512,
  })
  @IsOptional()
  @IsNumber()
  grp_attn_w?: number;

  @ApiProperty({
    description: 'Prevent system swapping of the model to disk in macOS',
    example: false,
  })
  @IsOptional()
  @IsBoolean()
  mlock?: boolean;

  @ApiProperty({
    description:
      'You can constrain the sampling using GBNF grammars by providing path to a grammar file',
  })
  @IsOptional()
  @IsString()
  grammar_file?: string;

  @ApiProperty({
    description: 'To enable Flash Attention, default is true',
    example: true,
  })
  @IsOptional()
  @IsBoolean()
  flash_attn?: boolean;

  @ApiProperty({
    description: 'KV cache type: f16, q8_0, q4_0, default is f16',
    example: 'f16',
  })
  @IsOptional()
  @IsString()
  cache_type?: string;

  @ApiProperty({
    description: 'To enable mmap, default is true',
    example: true,
  })
  @IsOptional()
  @IsBoolean()
  use_mmap?: boolean;

  @ApiProperty({
    description: 'The engine to use.',
    example: 'cortex.llamacpp',
  })
  @IsOptional()
  engine?: string;
}
