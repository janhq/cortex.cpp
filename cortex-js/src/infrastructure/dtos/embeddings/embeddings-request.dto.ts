import { Optional } from '@nestjs/common';
import { ApiProperty } from '@nestjs/swagger';

export class CreateEmbeddingsDto {
  @ApiProperty({
    description: 'Embedding model',
    type: String,
  })
  model: string;

  @ApiProperty({
    description:
      'Input text to embed, encoded as a string or array of tokens. To embed multiple inputs in a single request, pass an array of strings or array of token arrays.',
    type: [String],
  })
  input: string | string[];

  @ApiProperty({
    description:
      'Encoding format for the embeddings. Supported formats are float and int.',
    type: String,
  })
  @Optional()
  encoding_format?: string;

  @ApiProperty({
    description:
      'The number of dimensions the resulting output embeddings should have. Only supported in some models.',
    type: Number,
  })
  @Optional()
  dimensions?: number;
}
