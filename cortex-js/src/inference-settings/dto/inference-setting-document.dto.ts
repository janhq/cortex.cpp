import { IsString, ValidateNested } from 'class-validator';
import {
  ControllerProps,
  InferenceSettingDocument,
} from 'src/core/interfaces/inference-setting.interface';

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
  controllerProps: ControllerProps;
}
