import { IsString } from 'class-validator';
import { ApiProperty } from '@nestjs/swagger';

export class ChatCompletionMessage {
  @ApiProperty({ description: 'The Content of the chat message.' })
  @IsString()
  content: string;

  @ApiProperty({
    description: 'The role of the entity in the chat completion.',
  })
  role: 'user' | 'assistant';
}
