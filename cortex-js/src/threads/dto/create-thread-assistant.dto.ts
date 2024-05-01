import { IsArray, IsOptional, IsString, ValidateNested } from 'class-validator';
import { ThreadAssistantInfo } from 'src/core/interfaces/thread.interface';
import { CreateThreadModelInfoDto } from './create-thread-model-info.dto';
import { AssistantToolDto } from 'src/assistants/dto/assistant-tool.dto';
import { Type } from 'class-transformer';

export class CreateThreadAssistantDto implements ThreadAssistantInfo {
  @IsString()
  assistant_id: string;

  @IsString()
  assistant_name: string;

  @ValidateNested()
  model: CreateThreadModelInfoDto;

  @IsOptional()
  @IsString()
  instructions?: string;

  @IsOptional()
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => AssistantToolDto)
  tools?: AssistantToolDto[];
}
