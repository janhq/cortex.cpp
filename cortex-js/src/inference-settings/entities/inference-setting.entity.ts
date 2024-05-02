import {
  InferenceSetting,
  InferenceSettingDocument,
} from 'src/core/interfaces/inference-setting.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('inference_setting')
export class InferenceSettingEntity implements InferenceSetting {
  @PrimaryColumn()
  inferenceId: string;

  @Column({ type: 'simple-json' })
  settings: InferenceSettingDocument[];
}
