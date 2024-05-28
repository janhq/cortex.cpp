import { ApiProperty } from '@nestjs/swagger';
import { MessageDto } from './message.dto';

export class ChoiceDto {
  @ApiProperty({
    description: "The reason the chat completion ended, typically indicating whether the model completed the text naturally or was cut off.",
    type: String
  })
  finish_reason: string;
  
  @ApiProperty({
    description: "The index of the completion relative to other generated completions, useful for identifying its order in a batch request.",
    type: Number
  })
  index: number;
  
  @ApiProperty({
    description: "An object representing the message details involved in the chat completion, encapsulated within a MessageDto.",
    type: MessageDto
  })
  message: MessageDto;
}
