import { IsArray, IsNumber, IsOptional, IsString } from 'class-validator';
import { ModelMetadata } from '@/domain/models/model.interface';
import { ApiProperty } from '@nestjs/swagger';

export class ModelMetadataDto implements ModelMetadata {
  @ApiProperty({description: "The author of the model."})
  @IsString()
  author: string;

  @ApiProperty({description: "The model's tags."})
  @IsArray()
  tags: string[];

  @ApiProperty({description: "The model's size."})
  @IsNumber()
  size: number;

  @ApiProperty({description: "The model's cover."})
  @IsString()
  @IsOptional()
  cover?: string | undefined;
}
