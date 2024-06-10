import { ModelSettingParams } from '@/domain/models/model.interface';
import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsNumber, IsOptional, Min } from 'class-validator';

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
    example: 'cortex.llamacpp',
    description: 'The engine to use.',
  })
  @IsOptional()
  engine?: string;
}
