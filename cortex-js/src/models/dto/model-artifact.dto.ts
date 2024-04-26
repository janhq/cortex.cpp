import { IsString } from 'class-validator';
import { ModelArtifact } from 'src/core/interfaces/model.interface';

export class ModelArtifactDto implements ModelArtifact {
  @IsString()
  url: string;
}
