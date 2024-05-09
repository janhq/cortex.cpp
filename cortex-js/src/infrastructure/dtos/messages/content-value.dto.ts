import { IsArray, IsNumber, IsOptional, IsString } from 'class-validator';
import { ContentValue } from '@/domain/models/message.interface';

export class ContentValueDto implements ContentValue {
  @IsString()
  value: string;

  @IsArray()
  annotations: string[];

  @IsOptional()
  @IsString()
  name?: string;

  @IsOptional()
  @IsNumber()
  size?: number;
}
