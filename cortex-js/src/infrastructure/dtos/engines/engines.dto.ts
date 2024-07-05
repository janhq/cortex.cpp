import { Extension } from '@/domain/abstracts/extension.abstract';
import { ApiProperty } from '@nestjs/swagger';
import { IsOptional, IsString } from 'class-validator';

export class EngineDto implements Partial<Extension> {
  @ApiProperty({
    type: String,
    example: 'cortex.llamacpp',
    description:
      'The name of the engine that you want to retrieve.',
  })
  @IsString()
  name: string;

  // Prompt Settings
  @ApiProperty({
    type: String,
    example: 'Cortex',
    description: 'The display name of the engine.',
  })
  @IsString()
  @IsOptional()
  productName?: string;

  @ApiProperty({
    type: String,
    example: 'Cortex engine',
    description: 'The description of the engine.',
  })
  @IsString()
  @IsOptional()
  description?: string;

  @ApiProperty({
    type: String,
    example: '0.0.1',
    description: 'The version of the engine.',
  })
  @IsString()
  @IsOptional()
  version?: string;
}
