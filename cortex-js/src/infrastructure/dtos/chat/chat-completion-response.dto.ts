import { ApiProperty } from '@nestjs/swagger';
import { UsageDto } from './usage.dto';
import { ChoiceDto } from './choice.dto';

export class ChatCompletionResponseDto {
  @ApiProperty()
  choices: ChoiceDto[];

  @ApiProperty()
  created: number;

  @ApiProperty()
  id: string;

  @ApiProperty()
  model: string;

  @ApiProperty()
  object: string;

  @ApiProperty()
  system_fingerprint: string;

  @ApiProperty()
  usage: UsageDto;
}
