import { Extension } from '@/domain/abstracts/extension.abstract';
import { ApiProperty } from '@nestjs/swagger';
import { IsBoolean, IsOptional, IsString } from 'class-validator';

export class EngineDto implements Partial<Extension> {
  @ApiProperty({
    type: String,
    example: 'cortex.llamacpp',
    description: 'The name of the engine that you want to retrieve.',
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

  @ApiProperty({
    type: String,
    example: true,
    description: 'The status of the engine.',
  })
  @IsBoolean()
  status?: string;
}
export class InitEngineDto {
  @ApiProperty({
    type: String,
    example: 'CPU',
    description: 'The mode that you want to run the engine.',
  })
  @IsString()
  runMode?: 'CPU' | 'GPU';

  @ApiProperty({
    type: String,
    example: 'Nvidia',
    description: 'The type of GPU that you want to use.',
  })
  @IsString()
  gpuType?: 'Nvidia' | 'Others (Vulkan)';

  @ApiProperty({
    type: String,
    example: 'AVX',
    description: 'The instructions that you want to use.',
  })
  @IsString()
  instructions?: 'AVX' | 'AVX2' | 'AVX512' | undefined;

  @ApiProperty({
    type: String,
    example: '11',
    description: 'The version of CUDA that you want to use.',
  })
  @IsString()
  cudaVersion?: '11' | '12';

  @ApiProperty({
    type: Boolean,
    example: true,
    description: 'Silent mode.',
  })
  @IsBoolean()
  silent?: boolean;

  @ApiProperty({
    type: Boolean,
    example: true,
    description: 'Install Vulkan engine.',
  })
  @IsBoolean()
  vulkan?: boolean;

  @ApiProperty({
    type: String,
    example: true,
    description: 'Engine version.',
  })
  @IsBoolean()
  version?: string;
}
