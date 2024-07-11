import { VectorStoreEntity } from '@/infrastructure/entities/vector_store.entity';

import { Sequelize } from 'sequelize-typescript';

export const vectorStoreProviders = [
  {
    provide: 'VECTOR_STORE_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(VectorStoreEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];
