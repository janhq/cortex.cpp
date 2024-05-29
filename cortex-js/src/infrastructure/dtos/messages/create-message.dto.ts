import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsEnum, IsString, ValidateNested } from 'class-validator';
import {
  ChatCompletionRole,
  ErrorCode,
  Message,
  MessageStatus,
} from '@/domain/models/message.interface';
import { ThreadContentDto } from './thread-content.dto';
import { Type } from 'class-transformer';

export class CreateMessageDto implements Partial<Message> {
  @ApiProperty({description: "The ID of the thread to which the message will be posted."})
  @IsString()
  thread_id: string;

  @ApiProperty({description: "The assistant's unique identifier."})
  @IsString()
  assistant_id?: string;

  @ApiProperty({description: "The sources of the messages."})
  @IsEnum(ChatCompletionRole)
  role: ChatCompletionRole;

  @ApiProperty({description: "The content of the messages."})
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => ThreadContentDto)
  content: ThreadContentDto[];

  @ApiProperty({description: "Current status of the message."})
  @IsEnum(MessageStatus)
  status: MessageStatus;

  @ApiProperty({description: "Optional dictionary for additional unstructured message information."})
  metadata?: Record<string, unknown>;

  @ApiProperty({description: "Type of the message."})
  @IsString()
  type?: string;

  @ApiProperty({description: "Specifies the cause of any error."})
  @IsEnum(ErrorCode)
  error_code?: ErrorCode;
}
