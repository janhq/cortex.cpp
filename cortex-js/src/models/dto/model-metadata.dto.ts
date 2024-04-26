import { IsArray, IsNumber, IsOptional, IsString } from 'class-validator';
import { ModelMetadata } from 'src/core/interfaces/model.interface';

export class ModelMetadataDto implements ModelMetadata {
  @IsString()
  author: string;

  @IsArray()
  tags: string[];

  @IsNumber()
  size: number;

  @IsString()
  @IsOptional()
  cover?: string | undefined;
}
