import { IsEnum, IsString } from 'class-validator';
import { ChatCompletionRole } from 'src/domain/models/message.interface';

export class ChatCompletionMessage {
  @IsString()
  content: string;

  @IsEnum(ChatCompletionRole)
  role: ChatCompletionRole;
}
