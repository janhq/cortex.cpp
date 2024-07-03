console.time('threadProviders-import');
console.time('threadProviders-typeorm');
import { ThreadEntity } from '@/infrastructure/entities/thread.entity';
import { Sequelize } from 'sequelize-typescript';
console.timeEnd('threadProviders-typeorm');
console.timeEnd('threadProviders-import');

export const threadProviders = [
  {
    provide: 'THREAD_REPOSITORY',
    useFactory: async(sequelize: Sequelize) =>{
      return sequelize.getRepository(ThreadEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];

