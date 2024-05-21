import { ApiProperty } from '@nestjs/swagger';
import { MessageDto } from './message.dto';

export class ChoiceDto {
  @ApiProperty()
  finish_reason: string;

  @ApiProperty()
  index: number;

  @ApiProperty()
  message: MessageDto;
}
