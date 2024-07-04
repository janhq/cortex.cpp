import { ApiProperty } from '@nestjs/swagger';
import { IsOptional, IsString } from 'class-validator';

export class ConfigUpdateDto {
  @ApiProperty({
    example: 'apiKey',
    description: 'The configuration API key.',
  })
  @IsString()
  @IsOptional()
  key: string;

  // Prompt Settings
  @ApiProperty({
    type: String,
    example: 'sk-xxxxxx',
    description: 'The value of the configuration API key.',
  })
  @IsString()
  @IsOptional()
  value: string;

  @ApiProperty({
    type: String,
    example: 'openai',
    description: 'The name of the configuration.',
  })
  @IsString()
  @IsOptional()
  name?: string;
}
