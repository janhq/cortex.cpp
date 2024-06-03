import { IsArray, IsNumber, IsOptional, IsString } from 'class-validator';
import { ContentValue } from '@/domain/models/message.interface';
import { ApiProperty } from '@nestjs/swagger';

export class ContentValueDto implements ContentValue {
  @ApiProperty({ description: "The text's value." })
  @IsString()
  value: string;

  @ApiProperty({
    description: "The text's annotation that categorize the text.",
  })
  @IsArray()
  annotations: string[];

  @ApiProperty({ description: 'The name or title of the text.' })
  @IsOptional()
  @IsString()
  name?: string;

  @ApiProperty({ description: "The text's size in bytes." })
  @IsOptional()
  @IsNumber()
  size?: number;
}
