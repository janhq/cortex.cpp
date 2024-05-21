import { ApiProperty } from '@nestjs/swagger';

export class CortexOperationSuccessfullyDto {
  @ApiProperty({
    description: 'Cortex operation message.',
  })
  message: string;

  @ApiProperty({
    description: 'Cortex operation status.',
  })
  status: string;
}
