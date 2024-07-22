import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsString } from 'class-validator';
import { Message, MessageContent } from '@/domain/models/message.interface';

export class CreateMessageDto implements Partial<Message> {
  @ApiProperty({
    example: 'thread_456',
    description: 'The ID of the thread to which the message will be posted.',
  })
  @IsString()
  thread_id: string;

  @ApiProperty({
    example: 'assistant_789',
    description: "The assistant's unique identifier.",
  })
  @IsString()
  assistant_id?: string;

  @ApiProperty({
    example: 'user',
    description: 'The sources of the messages.',
  })
  @IsString()
  role: 'user' | 'assistant';

  @ApiProperty({
    example: [
      {
        type: 'text',
        data: 'Hello, how can I help you today?'
      }
    ],
    description: 'The content of the messages.',
  })
  @IsArray()
  content: MessageContent[];

  @ApiProperty({
    example: 'in_progress',
    description: 'Current status of the message.',
  })
  status: 'in_progress' | 'incomplete' | 'completed';

  @ApiProperty({
    example: { urgency: 'high', tags: ['customer_support'] },
    description: 'Optional dictionary for additional unstructured message information.',
  })
  metadata?: Record<string, unknown>;

  @ApiProperty({
    example: 'text',
    description: 'Type of the message.',
  })
  @IsString()
  type?: string;
}

