import { IsString } from 'class-validator';
import { ModelArtifact } from '@/domain/models/model.interface';
import { ApiProperty } from '@nestjs/swagger';

export class ModelArtifactDto implements ModelArtifact {
  @ApiProperty({description: "The URL source of the model."})
  @IsString()
  url: string;
}
