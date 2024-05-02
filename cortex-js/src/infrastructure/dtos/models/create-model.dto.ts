import { Type } from 'class-transformer';
import { IsArray, IsEnum, IsString, ValidateNested } from 'class-validator';
import {
  Model,
  ModelFormat,
} from 'src/domain/models/model.interface';
import { ModelArtifactDto } from './model-artifact.dto';
import { ModelSettingParamsDto } from './model-setting-params.dto';
import { ModelRuntimeParamsDto } from './model-runtime-params.dto';
import { ModelMetadataDto } from './model-metadata.dto';

export class CreateModelDto implements Partial<Model> {
  @IsString()
  version: string;

  @IsEnum(ModelFormat)
  format: ModelFormat;

  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => ModelArtifactDto)
  sources: ModelArtifactDto[];

  @IsString()
  id: string;

  @IsString()
  name: string;

  @IsString()
  description: string;

  @ValidateNested({ always: true, each: true })
  @Type(() => ModelSettingParamsDto)
  settings: ModelSettingParamsDto;

  @ValidateNested()
  parameters: ModelRuntimeParamsDto;

  @ValidateNested()
  metadata: ModelMetadataDto;

  @IsString()
  engine: string;
}
