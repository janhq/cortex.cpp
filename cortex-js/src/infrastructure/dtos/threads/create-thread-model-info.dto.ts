import { IsOptional, IsString, ValidateNested } from 'class-validator';
import { InferenceEngine, ModelInfo } from 'src/domain/models/model.interface';
import { ModelRuntimeParamsDto } from 'src/infrastructure/dtos/models/model-runtime-params.dto';
import { ModelSettingParamsDto } from 'src/infrastructure/dtos/models/model-setting-params.dto';

export class CreateThreadModelInfoDto implements ModelInfo {
  @IsString()
  id: string;

  @ValidateNested()
  settings: ModelSettingParamsDto;

  @ValidateNested()
  parameters: ModelRuntimeParamsDto;

  @IsOptional()
  @IsString()
  engine?: InferenceEngine;
}
