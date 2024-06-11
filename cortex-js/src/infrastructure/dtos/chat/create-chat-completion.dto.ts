import {
  IsArray,
  IsBoolean,
  IsNumber,
  IsOptional,
  IsString,
  ValidateNested,
} from 'class-validator';
import { ChatCompletionMessage } from './chat-completion-message.dto';
import { Type } from 'class-transformer';
import { ApiProperty } from '@nestjs/swagger';

export class CreateChatCompletionDto {
  @ApiProperty({
    description:
      'Array of chat messages to be used for generating the chat completion.',
  })
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => ChatCompletionMessage)
  messages: ChatCompletionMessage[];

  @ApiProperty({
    description: 'The unique identifier of the model.',
  })
  @IsString()
  model: string;

  @ApiProperty({
    description:
      'Determines the format for output generation. If set to `true`, the output is generated continuously, allowing for real-time streaming of responses. If set to `false`, the output is delivered in a single JSON file.',
  })
  @IsOptional()
  @IsBoolean()
  stream?: boolean;

  @ApiProperty({
    description:
      'Sets the upper limit on the number of tokens the model can generate in a single output.',
  })
  @IsOptional()
  @IsNumber()
  max_tokens?: number;

  @ApiProperty({
    description:
      'Defines specific tokens or phrases that signal the model to stop producing further output.',
  })
  @IsOptional()
  @IsArray()
  stop?: string[];

  @ApiProperty({
    description:
      'Modifies the likelihood of the model repeating the same words or phrases within a single output.',
  })
  @IsOptional()
  @IsNumber()
  frequency_penalty?: number;

  @ApiProperty({
    description:
      'Reduces the likelihood of repeating tokens, promoting novelty in the output.',
  })
  @IsOptional()
  @IsNumber()
  presence_penalty?: number;

  @ApiProperty({
    description: "Influences the randomness of the model's output.",
  })
  @IsOptional()
  @IsNumber()
  temperature?: number;

  @ApiProperty({
    description: 'Sets probability threshold for more relevant outputs.',
  })
  @IsOptional()
  @IsNumber()
  top_p?: number;
}
