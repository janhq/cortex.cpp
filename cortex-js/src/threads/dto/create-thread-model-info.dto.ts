import { IsEnum, IsString, ValidateNested } from 'class-validator';
import {
  InferenceEngine,
  ModelInfo,
} from 'src/core/interfaces/model.interface';
import { ModelRuntimeParamsDto } from 'src/models/dto/model-runtime-params.dto';
import { ModelSettingParamsDto } from 'src/models/dto/model-setting-params.dto';

export class CreateThreadModelInfoDto implements ModelInfo {
  @IsString()
  id: string;

  @ValidateNested()
  settings: ModelSettingParamsDto;

  @ValidateNested()
  parameters: ModelRuntimeParamsDto;

  @IsEnum(InferenceEngine)
  engine?: InferenceEngine;
}
