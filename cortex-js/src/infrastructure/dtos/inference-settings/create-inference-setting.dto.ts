import { Type } from 'class-transformer';
import { IsArray, IsString, ValidateNested } from 'class-validator';
import { InferenceSetting } from '@/domain/models/inference-setting.interface';
import { InferenceSettingDocumentDto } from './inference-setting-document.dto';

export class CreateInferenceSettingDto implements Partial<InferenceSetting> {
  @IsString()
  inferenceId: string;

  @IsArray()
  @ValidateNested({ each: true })
  @Type(() => InferenceSettingDocumentDto)
  settings: InferenceSettingDocumentDto[];
}
