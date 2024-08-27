import { ApiProperty } from '@nestjs/swagger';

export class ListMessageObjectDto {
  @ApiProperty({
    example: 'msg_abc123',
    description: 'The identifier of the message.',
  })
  id: string;

  @ApiProperty({
    example: 'thread.message',
    description: "Type of the object, indicating it's a thread message.",
  })
  object: string;

  @ApiProperty({
    example: 1699017614,
    description:
      'Unix timestamp representing the creation time of the message.',
    type: 'integer',
  })
  created_at: number;

  @ApiProperty({
    example: 'thread_abc123',
    description: 'Identifier of the thread to which this message belongs.',
  })
  thread_id: string;

  @ApiProperty({
    example: 'user',
    description: "Role of the sender, either 'user' or 'assistant'.",
  })
  role: string;

  @ApiProperty({
    description: 'Array of file IDs associated with the message, if any.',
    type: [String],
    example: [],
  })
  file_ids: string[];

  @ApiProperty({
    nullable: true,
    description:
      'Identifier of the assistant involved in the message, if applicable.',
    example: null,
  })
  assistant_id: string | null;

  @ApiProperty({
    nullable: true,
    description: 'Run ID associated with the message, if applicable.',
    example: null,
  })
  run_id: string | null;

  @ApiProperty({
    example: {},
    description: 'Metadata associated with the message.',
  })
  metadata: object;
}

export class ListMessagesResponseDto {
  @ApiProperty({
    example: 'list',
    description: "Type of the object, indicating it's a list.",
  })
  object: string;

  @ApiProperty({
    type: [ListMessageObjectDto],
    description: 'Array of message objects.',
  })
  data: ListMessageObjectDto[];

  @ApiProperty({
    example: 'msg_abc123',
    description: 'Identifier of the first message in the list.',
  })
  first_id: string;

  @ApiProperty({
    example: 'msg_abc456',
    description: 'Identifier of the last message in the list.',
  })
  last_id: string;

  @ApiProperty({
    example: false,
    description: 'Indicates whether there are more messages to retrieve.',
  })
  has_more: boolean;
}
