import { ApiProperty } from '@nestjs/swagger';
import { UsageDto } from './usage.dto';

export class EmbeddingsResponseDto {
  @ApiProperty({
    description: 'Type of the result object.',
    type: String,
  })
  object: string;

  @ApiProperty({
    description: 'Identifier of the model utilized for generating embeddings.',
    type: String,
  })
  model: string;

  @ApiProperty({
    description:
      'The embedding vector represented as an array of floating-point numbers. ',
    type: [Number],
  })
  embedding: [number];

  @ApiProperty({
    description:
      'Details of token usage, including prompt_tokens and total_tokens.',
    type: UsageDto,
  })
  usage: UsageDto;
}
