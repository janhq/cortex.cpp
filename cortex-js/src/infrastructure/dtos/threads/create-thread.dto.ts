import { IsArray } from 'class-validator';
import { Thread } from '@/domain/models/thread.interface';
import { CreateThreadAssistantDto } from './create-thread-assistant.dto';
import { ApiProperty } from '@nestjs/swagger';

export class CreateThreadDto implements Partial<Thread> {
  @ApiProperty({ description: "The details of the thread's settings." })
  @IsArray()
  assistants: CreateThreadAssistantDto[];
}
