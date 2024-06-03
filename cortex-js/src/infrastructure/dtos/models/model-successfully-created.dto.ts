import { Model } from '@/domain/models/model.interface';
import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsBoolean, IsNumber, IsOptional } from 'class-validator';

export class ModelDto implements Partial<Model> {
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
  })
  @IsOptional()
  @IsNumber()
  cpu_threads?: number;

  @ApiProperty({
    example: 'cortex.llamacpp',
    description: 'The engine to use.',
  })
  @IsOptional()
  engine?: string;
}
