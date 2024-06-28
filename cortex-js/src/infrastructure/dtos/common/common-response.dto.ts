import { ApiProperty } from '@nestjs/swagger';
import { IsString } from 'class-validator';

export class CommonResponseDto {
  @ApiProperty({
    description: 'The response success or error message.',
  })
  @IsString()
  message: string;
}
