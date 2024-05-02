import { PartialType } from '@nestjs/swagger';
import { CreateInferenceSettingDto } from './create-inference-setting.dto';

export class UpdateInferenceSettingDto extends PartialType(CreateInferenceSettingDto) {}
