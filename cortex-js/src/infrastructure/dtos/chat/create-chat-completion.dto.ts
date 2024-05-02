import {
  IsArray,
  IsBoolean,
  IsNumber,
  IsString,
  ValidateNested,
} from 'class-validator';
import { ChatCompletionMessage } from './chat-completion-message.dto';
import { Type } from 'class-transformer';

export class CreateChatCompletionDto {
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => ChatCompletionMessage)
  messages: ChatCompletionMessage[];

  @IsString()
  model: string;

  @IsBoolean()
  stream: boolean;

  @IsNumber()
  max_tokens: number;

  @IsArray()
  stop: string[];

  @IsNumber()
  frequency_penalty: number;

  @IsNumber()
  presence_penalty: number;

  @IsNumber()
  temperature: number;

  @IsNumber()
  top_p: number;
}
