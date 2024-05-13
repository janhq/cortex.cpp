import { IsOptional, IsString, ValidateNested } from 'class-validator';
import { ModelInfo } from '@/domain/models/model.interface';
import { ModelRuntimeParamsDto } from '@/infrastructure/dtos/models/model-runtime-params.dto';
import { ModelSettingParamsDto } from '@/infrastructure/dtos/models/model-setting-params.dto';

export class CreateThreadModelInfoDto implements ModelInfo {
  @IsString()
  id: string;

  @ValidateNested()
  settings: ModelSettingParamsDto;

  @ValidateNested()
  parameters: ModelRuntimeParamsDto;

  @IsOptional()
  @IsString()
  engine?: string;
}
