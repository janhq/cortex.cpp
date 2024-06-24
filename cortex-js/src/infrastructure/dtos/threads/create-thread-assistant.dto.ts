import { IsArray, IsNumber, IsOptional, IsString } from 'class-validator';
import { ApiProperty } from '@nestjs/swagger';
import type {
  Assistant,
  AssistantResponseFormatOption,
  AssistantToolResources,
} from '@/domain/models/assistant.interface';

export class CreateThreadAssistantDto implements Assistant {
  @ApiProperty({
    description: 'The unique identifier of the assistant.',
    type: 'string',
  })
  @IsString()
  id: string;

  @ApiProperty()
  @IsOptional()
  @IsString()
  avatar?: string;

  @ApiProperty({ description: 'The name of the assistant.' })
  @IsString()
  name: string;

  @ApiProperty({ description: "The model's unique identifier and settings." })
  @IsString()
  model: string;

  @ApiProperty({ description: "The assistant's specific instructions." })
  @IsString()
  instructions: string;

  @ApiProperty({
    description: "The thread's tool(Knowledge Retrieval) configurations.",
  })
  @IsOptional()
  @IsArray()
  tools: any;

  @ApiProperty()
  @IsString()
  @IsOptional()
  description: string | null;

  @ApiProperty()
  @IsOptional()
  metadata: Record<string, unknown> | null;

  @ApiProperty()
  object: 'assistant';

  @ApiProperty()
  @IsNumber()
  @IsOptional()
  temperature?: number | null;

  @ApiProperty()
  @IsNumber()
  @IsOptional()
  top_p?: number | null;

  @ApiProperty()
  created_at: number;

  @ApiProperty()
  @IsOptional()
  response_format?: AssistantResponseFormatOption;

  @ApiProperty()
  @IsOptional()
  tool_resources?: AssistantToolResources;
}
