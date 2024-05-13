import { Type } from 'class-transformer';
import { IsArray, IsOptional, IsString, ValidateNested } from 'class-validator';
import {
  Assistant,
  AssistantMetadata,
} from '@/domain/models/assistant.interface';
import { AssistantToolDto } from './assistant-tool.dto';

export class CreateAssistantDto implements Partial<Assistant> {
  @IsString()
  id: string;

  @IsString()
  avatar: string;

  @IsString()
  name: string;

  @IsString()
  description: string;

  @IsString()
  model: string;

  @IsString()
  instructions: string;

  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => AssistantToolDto)
  tools: AssistantToolDto[];

  @IsArray()
  @IsOptional()
  file_ids: string[];

  @IsOptional()
  metadata?: AssistantMetadata;
}
