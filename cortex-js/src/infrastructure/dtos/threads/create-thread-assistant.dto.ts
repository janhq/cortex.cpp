import { IsArray, IsNumber, IsOptional, IsString } from 'class-validator';
import { ApiProperty } from '@nestjs/swagger';
import type {
  Assistant,
  AssistantResponseFormatOption,
  AssistantToolResources,
} from '@/domain/models/assistant.interface';

export class CreateThreadAssistantDto implements Assistant {
  @ApiProperty({
    example: 'thread_123',
    description: 'The unique identifier of the assistant.',
    type: 'string',
  })
  @IsString()
  id: string;

  @ApiProperty({
    example: 'https://example.com/avatar.png',
    description: 'URL of the assistant\'s avatar image.',
    type: 'string',
  })
  @IsOptional()
  @IsString()
  avatar?: string;

  @ApiProperty({
    example: 'Virtual Helper',
    description: 'The name of the assistant.',
    type: 'string',
  })
  @IsString()
  name: string;

  @ApiProperty({
    example: 'mistral',
    description: 'The model\'s unique identifier and settings.',
    type: 'string',
  })
  @IsString()
  model: string;

  @ApiProperty({
    example: 'Assist with customer queries and provide information based on the company database.',
    description: 'The assistant\'s specific instructions.',
    type: 'string',
  })
  @IsString()
  instructions: string;

  @ApiProperty({
    example: [{ name: 'Knowledge Retrieval', settings: { source: 'internal', endpoint: 'https://api.example.com/knowledge' } }],
    description: 'The thread\'s tool(Knowledge Retrieval) configurations.',
    type: 'array',
  })
  @IsOptional()
  @IsArray()
  tools: any;

  @ApiProperty({
    example: 'This assistant helps with customer support by retrieving relevant information.',
    description: 'The description of the assistant.',
    type: 'string',
  })
  @IsString()
  @IsOptional()
  description: string | null;

  @ApiProperty({
    example: { department: 'support', version: '1.0' },
    description: 'Additional metadata for the assistant.',
    type: 'object',
  })
  @IsOptional()
  metadata: Record<string, unknown> | null;

  @ApiProperty({
    example: 'assistant',
    description: 'The object type, always "assistant".',
    type: 'string',
  })
  object: 'assistant';

  @ApiProperty({
    example: 0.7,
    description: 'Sampling temperature for the assistant.',
    type: 'number',
  })
  @IsNumber()
  @IsOptional()
  temperature?: number | null;

  @ApiProperty({
    example: 0.9,
    description: 'Top-p sampling value for the assistant.',
    type: 'number',
  })
  @IsNumber()
  @IsOptional()
  top_p?: number | null;

  @ApiProperty({
    example: 1622470423,
    description: 'Timestamp of when the assistant was created.',
    type: 'number',
  })
  created_at: number;

  @ApiProperty({
    example: { format: 'json' },
    description: 'The response format option for the assistant.',
    type: 'object',
  })
  @IsOptional()
  response_format?: AssistantResponseFormatOption;

  @ApiProperty({
    example: { resources: ['database1', 'database2'] },
    description: 'Tool resources for the assistant.',
    type: 'object',
  })
  @IsOptional()
  tool_resources?: AssistantToolResources;
}