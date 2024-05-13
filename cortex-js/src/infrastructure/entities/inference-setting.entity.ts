import {
  InferenceSetting,
  InferenceSettingDocument,
} from '@/domain/models/inference-setting.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('inference_setting')
export class InferenceSettingEntity implements InferenceSetting {
  @PrimaryColumn()
  inferenceId: string;

  @Column({ type: 'simple-json' })
  settings: InferenceSettingDocument[];
}
