import { Optional } from '@nestjs/common';
import { ApiProperty } from '@nestjs/swagger';

export class CreateEmbeddingsDto {
  @ApiProperty({
    example: 'llama3',
    description: 'The name of the embedding model to be used.',
    type: String,
  })
  model: string;

  @ApiProperty({
    example: ['Hello World'],
    description:
      'The text or token array(s) to be embedded. This can be a single string, an array of strings, or an array of token arrays to embed multiple inputs in one request.',
    type: [String],
  })
  input: string | string[];

  @ApiProperty({
    example: 'float',
    description:
      'Specifies the format for the embeddings. Supported formats include `float` and `int`. This field is optional.',
    type: String,
  })
  @Optional()
  encoding_format?: string;

  @ApiProperty({
    example: 3,
    description:
      'Defines the number of dimensions for the output embeddings. This feature is supported by certain models only. This field is optional.',
    type: Number,
  })
  @Optional()
  dimensions?: number;
}
