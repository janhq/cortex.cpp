import { IsBoolean, IsNumber, IsOptional, IsString } from 'class-validator';
import { ModelSettingParams } from 'src/domain/models/model.interface';

export class ModelSettingParamsDto implements ModelSettingParams {
  @IsOptional()
  @IsNumber()
  ctx_len?: number;

  @IsOptional()
  @IsNumber()
  ngl?: number;

  @IsOptional()
  @IsBoolean()
  embedding?: boolean;

  @IsOptional()
  @IsNumber()
  n_parallel?: number;

  @IsOptional()
  @IsNumber()
  cpu_threads?: number;

  @IsOptional()
  @IsString()
  prompt_template?: string;

  @IsOptional()
  @IsString()
  system_prompt?: string;

  @IsOptional()
  @IsString()
  ai_prompt?: string;

  @IsOptional()
  @IsString()
  user_prompt?: string;

  @IsOptional()
  @IsString()
  llama_model_path?: string;

  @IsOptional()
  @IsString()
  mmproj?: string;

  @IsOptional()
  @IsBoolean()
  cont_batching?: boolean;

  @IsOptional()
  @IsBoolean()
  vision_model?: boolean;

  @IsOptional()
  @IsBoolean()
  text_model?: boolean;
}
