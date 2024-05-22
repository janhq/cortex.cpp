import { ApiProperty } from '@nestjs/swagger';

class Usage {
  @ApiProperty({
    type: Number,
    description: 'Number of tokens in the generated completion.',
  })
  completion_tokens: number;

  @ApiProperty({ type: Number, description: 'Number of tokens in the prompt.' })
  prompt_tokens: number;

  @ApiProperty({
    type: Number,
    description:
      'Total number of tokens used in the request (prompt + completion).',
  })
  total_tokens: number;
}

class Choice {
  @ApiProperty({ type: String, description: 'The text of the completion.' })
  text: string;

  @ApiProperty({ type: Number, description: 'Index of the choice.' })
  index: number;

  @ApiProperty({
    type: String,
    description: 'Reason for finishing the completion.',
  })
  finish_reason: string;
}

export class ChatCompletionResponse {
  @ApiProperty({
    type: String,
    description: 'A unique identifier for the chat completion.',
  })
  id: string;

  @ApiProperty({
    type: [Choice],
    description: 'A list of chat completion choices.',
  })
  choices: Choice[];

  @ApiProperty({
    type: Number,
    description:
      'The Unix timestamp (in seconds) of when the chat completion was created.',
  })
  created: number;

  @ApiProperty({
    type: String,
    description: 'The model used for the chat completion.',
  })
  model: string;

  @ApiProperty({
    type: String,
    description:
      'The system fingerprint representing the backend configuration.',
  })
  system_fingerprint: string;

  @ApiProperty({
    type: String,
    description: 'The object type, which is always chat.completion.',
  })
  object: string;

  @ApiProperty({
    type: Usage,
    description: 'Usage statistics for the completion request.',
  })
  usage: Usage;
}

export class ChatCompletionChunkedResponse {
  @ApiProperty({
    type: String,
    description:
      'A unique identifier for the chat completion. Each chunk has the same ID.',
  })
  id: string;

  @ApiProperty({
    type: [Choice],
    description: 'A list of chat completion choices.',
  })
  choices: Choice[];

  @ApiProperty({
    type: Number,
    description:
      'The Unix timestamp (in seconds) of when the chat completion was created. Each chunk has the same timestamp.',
  })
  created: number;

  @ApiProperty({
    type: String,
    description: 'The model used to generate the completion.',
  })
  model: string;

  @ApiProperty({
    type: String,
    description:
      'The system fingerprint representing the backend configuration.',
  })
  system_fingerprint: string;

  @ApiProperty({
    type: String,
    description: 'The object type, which is always chat.completion.chunk.',
  })
  object: string;

  @ApiProperty({
    type: Usage,
    required: false,
    description: 'Usage statistics for the completion request.',
  })
  usage?: Usage;
}
