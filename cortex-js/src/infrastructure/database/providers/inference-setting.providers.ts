import { InferenceSettingEntity } from '@/infrastructure/entities/inference-setting.entity';
import { DataSource } from 'typeorm';

export const inferenceSettingProviders = [
  {
    provide: 'INFERENCE_SETTING_REPOSITORY',
    useFactory: (dataSource: DataSource) =>
      dataSource.getRepository(InferenceSettingEntity),
    inject: ['DATA_SOURCE'],
  },
];
