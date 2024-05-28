import { Type } from 'class-transformer';
import { IsArray, IsOptional, IsString, ValidateNested } from 'class-validator';
import {
  Assistant,
  AssistantMetadata,
} from '@/domain/models/assistant.interface';
import { AssistantToolDto } from './assistant-tool.dto';
import { ApiProperty, ApiPropertyOptional } from '@nestjs/swagger';

export class CreateAssistantDto implements Partial<Assistant> {
  @ApiProperty({
    description: 'The unique identifier of the assistant.',
  })
  @IsString()
  id: string;

  @ApiProperty({
    description: 'The avatar of the assistant.',
  })
  @IsString()
  avatar: string;

  @ApiProperty({
    description: 'The name of the assistant.',
  })
  @IsString()
  name: string;

  @ApiProperty({
    description: 'The description of the assistant.',
  })
  @IsString()
  description: string;

  @ApiProperty({
    description: 'The model of the assistant.',
  })
  @IsString()
  model: string;

  @ApiProperty({
    description: 'The instructions for the assistant.',
  })
  @IsString()
  instructions: string;

  @ApiProperty({
    description: 'The tools associated with the assistant.',
  })
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => AssistantToolDto)
  tools: AssistantToolDto[];

  @ApiProperty({
    description:
      'The identifiers of the files that have been uploaded to the thread.',
  })
  @IsArray()
  @IsOptional()
  file_ids: string[];

  @ApiProperty({
    description: 'The metadata of the assistant.',
  })
  @IsOptional()
  metadata?: AssistantMetadata;
}
