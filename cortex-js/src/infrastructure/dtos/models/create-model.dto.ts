import { Type } from 'class-transformer';
import { IsArray, IsEnum, IsString, ValidateNested } from 'class-validator';
import { Model, ModelFormat } from '@/domain/models/model.interface';
import { ModelArtifactDto } from './model-artifact.dto';
import { ModelSettingParamsDto } from './model-setting-params.dto';
import { ModelRuntimeParamsDto } from './model-runtime-params.dto';
import { ModelMetadataDto } from './model-metadata.dto';
import { ApiProperty } from '@nestjs/swagger';

export class CreateModelDto implements Partial<Model> {
  @ApiProperty({ description: 'The version of the model.' })
  @IsString()
  version: string;

  @ApiProperty({ description: 'The state format of the model.' })
  @IsEnum(ModelFormat)
  format: ModelFormat;

  @ApiProperty({
    description: 'The URL sources from which the model downloaded or accessed.',
  })
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => ModelArtifactDto)
  sources: ModelArtifactDto[];

  @ApiProperty({ description: 'The unique identifier of the model.' })
  @IsString()
  id: string;

  @ApiProperty({ description: 'The name of the model.' })
  @IsString()
  name: string;

  @ApiProperty({ description: 'A brief description of the model.' })
  @IsString()
  description: string;

  @ApiProperty({ description: 'The settings parameters of the model.' })
  @ValidateNested({ always: true, each: true })
  @Type(() => ModelSettingParamsDto)
  settings: ModelSettingParamsDto;

  @ApiProperty({ description: 'The parameters configuration of the model.' })
  @ValidateNested()
  parameters: ModelRuntimeParamsDto;

  @ApiProperty({ description: 'The metadata of the model.' })
  @ValidateNested()
  metadata: ModelMetadataDto;

  @ApiProperty({ description: 'The engine used to run the model.' })
  @IsString()
  engine: string;
}
