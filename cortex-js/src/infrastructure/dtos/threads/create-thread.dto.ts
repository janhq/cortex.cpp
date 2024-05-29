import { IsArray, IsOptional, IsString, ValidateNested } from 'class-validator';
import { Thread } from '@/domain/models/thread.interface';
import { CreateThreadAssistantDto } from './create-thread-assistant.dto';
import { Type } from 'class-transformer';
import { ApiProperty } from '@nestjs/swagger';

export class CreateThreadDto implements Partial<Thread> {
  @ApiProperty({description: "The title of the thread."})
  @IsOptional()
  @IsString()
  title: string;

  @ApiProperty({description: "The details of the thread's settings."})
  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => CreateThreadAssistantDto)
  assistants: CreateThreadAssistantDto[];
}
