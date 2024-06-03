import { IsBoolean, IsNumber, IsOptional, IsString } from 'class-validator';
import { ModelSettingParams } from '@/domain/models/model.interface';
import { ApiProperty } from '@nestjs/swagger';

export class ModelSettingParamsDto implements ModelSettingParams {
  @ApiProperty({
    description:
      'Sets the maximum input the model can use to generate a response, it varies with the model used.',
  })
  @IsOptional()
  @IsNumber()
  ctx_len?: number;

  @ApiProperty({ description: 'Determines GPU layer usage.' })
  @IsOptional()
  @IsNumber()
  ngl?: number;

  @ApiProperty({
    description:
      'Enables embedding utilization for tasks like document-enhanced chat in RAG-based applications.',
  })
  @IsOptional()
  @IsBoolean()
  embedding?: boolean;

  @ApiProperty({ description: 'Number of parallel processing units to use.' })
  @IsOptional()
  @IsNumber()
  n_parallel?: number;

  @ApiProperty({
    description:
      'Determines CPU inference threads, limited by hardware and OS. ',
  })
  @IsOptional()
  @IsNumber()
  cpu_threads?: number;

  @ApiProperty({
    description:
      "A predefined text or framework that guides the AI model's response generation.",
  })
  @IsOptional()
  @IsString()
  prompt_template?: string;

  @ApiProperty({
    description:
      'Specific prompt used by the system for generating model outputs.',
  })
  @IsOptional()
  @IsString()
  system_prompt?: string;

  @ApiProperty({
    description:
      'The prompt fed into the AI, typically to guide or specify the nature of the content it should generate.',
  })
  @IsOptional()
  @IsString()
  ai_prompt?: string;

  @ApiProperty({
    description:
      'Customizable prompt input by the user to direct the model’s output generation.',
  })
  @IsOptional()
  @IsString()
  user_prompt?: string;

  @ApiProperty({ description: 'File path to a specific llama model.' })
  @IsOptional()
  @IsString()
  llama_model_path?: string;

  @ApiProperty({
    description:
      'The mmproj is a projection matrix that is used to project the embeddings from CLIP into tokens usable by llama/mistral.',
  })
  @IsOptional()
  @IsString()
  mmproj?: string;

  @ApiProperty({
    description:
      'Controls continuous batching, enhancing throughput for LLM inference.',
  })
  @IsOptional()
  @IsBoolean()
  cont_batching?: boolean;

  @ApiProperty({
    description:
      'Specifies if a vision-based model (for image processing) should be used.',
  })
  @IsOptional()
  @IsBoolean()
  vision_model?: boolean;

  @ApiProperty({
    description:
      'Specifies if a text-based model is to be utilized, for tasks like text generation or analysis.',
  })
  @IsOptional()
  @IsBoolean()
  text_model?: boolean;
}
