import { IsEnum, ValidateNested } from 'class-validator';
import { ContentType, ThreadContent } from '@/domain/models/message.interface';
import { ContentValueDto } from './content-value.dto';
import { ApiProperty } from '@nestjs/swagger';

export class ThreadContentDto implements ThreadContent {
  @ApiProperty({description: "The type of content."})
  @IsEnum(ContentType)
  type: ContentType;

  @ApiProperty({description: "The content details."})
  @ValidateNested()
  text: ContentValueDto;
}
