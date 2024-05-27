import { ApiProperty } from '@nestjs/swagger';

export class UsageDto {
  @ApiProperty()
  completion_tokens: number;

  @ApiProperty()
  prompt_tokens: number;

  @ApiProperty()
  total_tokens: number;
}
