import { ThreadEntity } from '@/infrastructure/entities/thread.entity';
import { Sequelize } from 'sequelize-typescript';

export const threadProviders = [
  {
    provide: 'THREAD_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(ThreadEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];
