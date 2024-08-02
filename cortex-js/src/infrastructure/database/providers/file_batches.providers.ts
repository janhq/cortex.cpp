import { FileBatchEntity } from '@/infrastructure/entities/file_batch.entity';

import { Sequelize } from 'sequelize-typescript';

export const fileBatchesProviders = [
  {
    provide: 'FILE_BATCH_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(FileBatchEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];
