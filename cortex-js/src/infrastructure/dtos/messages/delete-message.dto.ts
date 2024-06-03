import { ApiProperty } from '@nestjs/swagger';

export class DeleteMessageResponseDto {
  @ApiProperty({
    example: 'message_123',
    description: 'The identifier of the message that was deleted.',
  })
  id: string;

  @ApiProperty({
    example: 'message',
    description: "Type of the object, indicating it's a message.",
    default: 'message',
  })
  object: string;

  @ApiProperty({
    example: true,
    description: 'Indicates whether the message was successfully deleted.',
  })
  deleted: boolean;
}
