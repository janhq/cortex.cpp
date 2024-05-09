import {
  IsArray,
  IsBoolean,
  IsNumber,
  IsOptional,
  IsString,
} from 'class-validator';
import { ModelRuntimeParams } from '@/domain/models/model.interface';

export class ModelRuntimeParamsDto implements ModelRuntimeParams {
  @IsOptional()
  @IsNumber()
  temperature?: number;

  @IsOptional()
  @IsNumber()
  token_limit?: number;

  @IsOptional()
  @IsNumber()
  top_k?: number;

  @IsOptional()
  @IsNumber()
  top_p?: number;

  @IsOptional()
  @IsBoolean()
  stream?: boolean;

  @IsOptional()
  @IsNumber()
  max_tokens?: number;

  @IsOptional()
  @IsArray()
  stop?: string[];

  @IsOptional()
  @IsNumber()
  frequency_penalty?: number;

  @IsOptional()
  @IsNumber()
  presence_penalty?: number;

  @IsOptional()
  @IsString()
  engine?: string;
}
