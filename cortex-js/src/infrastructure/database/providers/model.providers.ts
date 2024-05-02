import { ModelEntity } from 'src/infrastructure/entities/model.entity';
import { DataSource } from 'typeorm';

export const modelProviders = [
  {
    provide: 'MODEL_REPOSITORY',
    useFactory: (dataSource: DataSource) =>
      dataSource.getRepository(ModelEntity),
    inject: ['DATA_SOURCE'],
  },
];
