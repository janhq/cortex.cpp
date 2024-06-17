import { LlmEngine, ModelSettingParams } from '@/domain/models/model.interface';
import { ApiProperty } from '@nestjs/swagger';
import {
  IsArray,
  IsBoolean,
  IsNumber,
  IsOptional,
  IsString,
  Min,
} from 'class-validator';

export class ModelSettingsDto implements ModelSettingParams {
  // Prompt Settings
  @ApiProperty({
    example: 'system\n{system_message}\nuser\n{prompt}\nassistant',
    description:
      "A predefined text or framework that guides the AI model's response generation.",
  })
  @IsOptional()
  prompt_template?: string;

  @ApiProperty({
    type: [String],
    example: [],
    description:
      'Defines specific tokens or phrases that signal the model to stop producing further output.',
  })
  @IsArray()
  @IsOptional()
  stop?: string[];

  // Engine Settings
  @ApiProperty({ description: 'Determines GPU layer usage.', example: 4096 })
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
      'Determines CPU inference threads, limited by hardware and OS. ',
    example: 10,
  })
  @IsOptional()
  @IsNumber()
  @Min(1)
  cpu_threads?: number;

  @ApiProperty({
    description: 'The prompt to use for internal configuration',
  })
  @IsOptional()
  @IsString()
  pre_prompt?: string;

  @ApiProperty({
    description: 'The batch size for prompt eval step',
    example: 2048,
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
    example: 'cortex.llamacpp',
    description: 'The engine to use.',
  })
  @IsOptional()
  engine?: LlmEngine;
}
