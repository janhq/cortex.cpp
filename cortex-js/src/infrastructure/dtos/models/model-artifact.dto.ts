import { IsString } from 'class-validator';
import { ModelArtifact } from '@/domain/models/model.interface';

export class ModelArtifactDto implements ModelArtifact {
  @IsString()
  url: string;
}
