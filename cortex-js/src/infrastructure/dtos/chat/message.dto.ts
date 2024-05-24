import { ApiProperty } from '@nestjs/swagger';

export class MessageDto {
  @ApiProperty()
  content: string;

  @ApiProperty()
  role: string;
}
