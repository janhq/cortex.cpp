import { IsString, ValidateNested } from 'class-validator';
import { InferenceSettingDocument } from '@/domain/models/inference-setting.interface';
import { ControllerPropsDto } from './controller-props.dto';

export class InferenceSettingDocumentDto implements InferenceSettingDocument {
  @IsString()
  key: string;

  @IsString()
  extensionName: string;

  @IsString()
  title: string;

  @IsString()
  description: string;

  @IsString()
  controllerType: string;

  @ValidateNested()
  controllerProps: ControllerPropsDto;
}
