import {
  IsArray,
  IsBoolean,
  IsNumber,
  IsOptional,
  IsString,
  Min,
} from 'class-validator';
import { Model } from '@/domain/models/model.interface';
import { ModelArtifactDto } from './model-artifact.dto';
import { ApiProperty, getSchemaPath } from '@nestjs/swagger';

export class CreateModelDto implements Partial<Model> {
  // Cortex Meta
  @ApiProperty({ description: 'The unique identifier of the model.' })
  @IsString()
  model: string;

  @ApiProperty({ description: 'The name of the model.' })
  @IsString()
  name?: string;

  @ApiProperty({
    description: 'The URL sources from which the model downloaded or accessed.',
    oneOf: [
      { type: 'array', items: { type: 'string' } },
      { $ref: getSchemaPath(ModelArtifactDto) },
    ],
  })
  @IsArray()
  files: string[] | ModelArtifactDto;

  // Model Input / Output Syntax
  @ApiProperty({
    description:
      "A predefined text or framework that guides the AI model's response generation.",
  })
  @IsOptional()
  @IsString()
  prompt_template?: string;

  @ApiProperty({
    description:
      'Defines specific tokens or phrases that signal the model to stop producing further output.',
  })
  @IsOptional()
  @IsArray()
  stop?: string[];

  // Results Preferences
  @ApiProperty({
    description:
      'Sets the upper limit on the number of tokens the model can generate in a single output.',
    example: 4096,
  })
  @IsOptional()
  @IsNumber()
  max_tokens?: number;

  @ApiProperty({
    description: 'Sets probability threshold for more relevant outputs.',
  })
  @IsOptional()
  @IsNumber()
  top_p?: number;

  @ApiProperty({
    description: "Influences the randomness of the model's output.",
  })
  @IsOptional()
  @IsNumber()
  temperature?: number;

  @ApiProperty({
    description:
      'Modifies the likelihood of the model repeating the same words or phrases within a single output.',
  })
  @IsOptional()
  @IsNumber()
  frequency_penalty?: number;

  @ApiProperty({
    description:
      'Reduces the likelihood of repeating tokens, promoting novelty in the output.',
  })
  @IsOptional()
  @IsNumber()
  presence_penalty?: number;

  @ApiProperty({
    description:
      'Determines the format for output generation. If set to `true`, the output is generated continuously, allowing for real-time streaming of responses. If set to `false`, the output is delivered in a single JSON file.',
  })
  @IsOptional()
  @IsBoolean()
  stream?: boolean;

  // Engine Settings
  @ApiProperty({
    description:
      'Sets the maximum input the model can use to generate a response, it varies with the model used.',
    example: 4096,
  })
  @IsOptional()
  @IsNumber()
  ctx_len?: number;

  @ApiProperty({ description: 'Determines GPU layer usage.', example: 32 })
  @IsOptional()
  @IsNumber()
  ngl?: number;

  @ApiProperty({
    description: 'Number of parallel processing units to use.',
    example: 1,
  })
  @IsOptional()
  @IsNumber()
  @Min(1)
  n_parallel?: number;

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
    description: 'The engine used to run the model.',
    example: 'cortex.llamacpp',
  })
  @IsOptional()
  @IsString()
  engine?: string;
}
