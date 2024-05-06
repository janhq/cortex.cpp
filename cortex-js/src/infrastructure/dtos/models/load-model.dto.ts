import { IsOptional, IsString, ValidateNested } from 'class-validator';
import { ModelSettingParamsDto } from './model-setting-params.dto';

export class LoadModelDto {
  @IsString()
  modelId: string;

  @IsOptional()
  @ValidateNested()
  settings: ModelSettingParamsDto;
}
