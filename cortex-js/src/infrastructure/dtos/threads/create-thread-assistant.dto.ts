import { IsArray, IsOptional, IsString, ValidateNested } from 'class-validator';
import { ThreadAssistantInfo } from '@/domain/models/thread.interface';
import { AssistantToolDto } from '@/infrastructure/dtos/assistants/assistant-tool.dto';
import { Type } from 'class-transformer';
import { ApiProperty } from '@nestjs/swagger';
import { CreateModelDto } from '../models/create-model.dto';

export class CreateThreadAssistantDto implements ThreadAssistantInfo {
  @ApiProperty({ description: 'The unique identifier of the assistant.' })
  @IsString()
  assistant_id: string;

  @ApiProperty({ description: 'The name of the assistant.' })
  @IsString()
  assistant_name: string;

  @ApiProperty({ description: "The model's unique identifier and settings." })
  @ValidateNested()
  model: CreateModelDto;

  @ApiProperty({ description: "The assistant's specific instructions." })
  @IsOptional()
  @IsString()
  instructions?: string;

  @ApiProperty({
    description: "The thread's tool(Knowledge Retrieval) configurations.",
  })
  @IsOptional()
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => AssistantToolDto)
  tools?: AssistantToolDto[];
}
