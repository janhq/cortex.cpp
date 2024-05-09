import { IsArray, IsOptional, IsString, ValidateNested } from 'class-validator';
import { Thread } from '@/domain/models/thread.interface';
import { CreateThreadAssistantDto } from './create-thread-assistant.dto';
import { Type } from 'class-transformer';

export class CreateThreadDto implements Partial<Thread> {
  @IsOptional()
  @IsString()
  title: string;

  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => CreateThreadAssistantDto)
  assistants: CreateThreadAssistantDto[];
}
