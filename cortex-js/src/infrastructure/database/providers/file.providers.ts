import { FileEntity } from '@/infrastructure/entities/file.entity';

import { Sequelize } from 'sequelize-typescript';

export const fileProviders = [
  {
    provide: 'FILE_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(FileEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];
