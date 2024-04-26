import { IsEnum, ValidateNested } from 'class-validator';
import {
  ContentType,
  ThreadContent,
} from 'src/core/interfaces/message.interface';
import { ContentValueDto } from './content-value.dto';

export class ThreadContentDto implements ThreadContent {
  @IsEnum(ContentType)
  type: ContentType;

  @ValidateNested()
  text: ContentValueDto;
}
