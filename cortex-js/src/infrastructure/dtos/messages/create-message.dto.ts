import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsString } from 'class-validator';
import { Message, MessageContent } from '@/domain/models/message.interface';

export class CreateMessageDto implements Partial<Message> {
  @ApiProperty({
    description: 'The ID of the thread to which the message will be posted.',
  })
  @IsString()
  thread_id: string;

  @ApiProperty({ description: "The assistant's unique identifier." })
  @IsString()
  assistant_id?: string;

  @ApiProperty({ description: 'The sources of the messages.' })
  role: 'user' | 'assistant';

  @ApiProperty({ description: 'The content of the messages.' })
  @IsArray()
  content: MessageContent[];

  @ApiProperty({ description: 'Current status of the message.' })
  status: 'in_progress' | 'incomplete' | 'completed';

  @ApiProperty({
    description:
      'Optional dictionary for additional unstructured message information.',
  })
  metadata?: Record<string, unknown>;

  @ApiProperty({ description: 'Type of the message.' })
  @IsString()
  type?: string;
}
