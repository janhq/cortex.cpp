import { IsString } from 'class-validator';
import { ModelArtifact } from 'src/domain/models/model.interface';

export class ModelArtifactDto implements ModelArtifact {
  @IsString()
  url: string;
}
