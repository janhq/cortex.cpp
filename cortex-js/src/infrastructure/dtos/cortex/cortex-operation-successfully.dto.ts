import { ApiProperty } from '@nestjs/swagger';

export class CortexOperationSuccessfullyDto {
  @ApiProperty({
    description: 'The cortex operation status message.',
  })
  message: string;

  @ApiProperty({
    description: 'The cortex operation status code.',
  })
  status: string;
}
