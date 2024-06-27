import { ApiProperty } from '@nestjs/swagger';
import { IsString } from 'class-validator';

export class CommonResponseDto {
  @ApiProperty({
    description: 'The success or error message',
  })
  @IsString()
  message: string;
}
