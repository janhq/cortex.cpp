import { ApiProperty } from '@nestjs/swagger';
import { UsageDto } from './usage.dto';

export class EmbeddingsResponseDto {
  @ApiProperty({
    description: 'Result object type.',
    type: String,
  })
  object: string;

  @ApiProperty({
    description: 'ID of the model used for embeddings',
    type: String,
  })
  model: string;

  @ApiProperty({
    description: 'The embedding vector, which is a list of floats. ',
    type: [Number],
  })
  embedding: [number];

  @ApiProperty({
    description: 'Returns prompt_tokens and total_tokens usage ',
    type: UsageDto,
  })
  usage: UsageDto;
}
