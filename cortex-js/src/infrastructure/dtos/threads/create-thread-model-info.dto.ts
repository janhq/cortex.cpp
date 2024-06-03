import { IsOptional, IsString, ValidateNested } from 'class-validator';
import { ModelInfo } from '@/domain/models/model.interface';
import { ModelRuntimeParamsDto } from '@/infrastructure/dtos/models/model-runtime-params.dto';
import { ModelSettingParamsDto } from '@/infrastructure/dtos/models/model-setting-params.dto';
import { ApiProperty } from '@nestjs/swagger';

export class CreateThreadModelInfoDto implements ModelInfo {
  @ApiProperty({ description: 'The unique identifier of the thread.' })
  @IsString()
  id: string;

  @ApiProperty({ description: 'The settings of the thread.' })
  @ValidateNested()
  settings: ModelSettingParamsDto;

  @ApiProperty({ description: 'The parameters of the thread.' })
  @ValidateNested()
  parameters: ModelRuntimeParamsDto;

  @ApiProperty({
    description: 'The engine used in the thread to operate the model.',
  })
  @IsOptional()
  @IsString()
  engine?: string;
}
