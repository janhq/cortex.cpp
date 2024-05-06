import { ApiProperty } from '@nestjs/swagger';

export class CortexOperationSuccessfullyDto {
  @ApiProperty()
  message: string;

  @ApiProperty()
  status: string;
}
