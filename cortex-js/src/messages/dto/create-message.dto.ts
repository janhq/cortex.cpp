import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsEnum, IsString, ValidateNested } from 'class-validator';
import {
  ChatCompletionRole,
  ErrorCode,
  Message,
  MessageStatus,
} from 'src/core/interfaces/message.interface';
import { ThreadContentDto } from './thread-content.dto';
import { Type } from 'class-transformer';

export class CreateMessageDto implements Partial<Message> {
  @IsString()
  thread_id: string;

  @IsString()
  assistant_id?: string;

  @IsEnum(ChatCompletionRole)
  role: ChatCompletionRole;

  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => ThreadContentDto)
  content: ThreadContentDto[];

  @IsEnum(MessageStatus)
  status: MessageStatus;

  @ApiProperty()
  metadata?: Record<string, unknown>;

  @IsString()
  type?: string;

  @IsEnum(ErrorCode)
  error_code?: ErrorCode;
}
