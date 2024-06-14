import { ApiProperty } from '@nestjs/swagger';

export default class DeleteMessageDto {
  @ApiProperty()
  id: string;

  @ApiProperty()
  object: string;

  @ApiProperty()
  deleted: boolean;
}
