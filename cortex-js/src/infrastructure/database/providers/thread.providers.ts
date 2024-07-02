console.time('threadProviders-import');
console.time('threadProviders-typeorm');
import { DataSource } from 'typeorm';
console.timeEnd('threadProviders-typeorm');
console.timeEnd('threadProviders-import');

export const threadProviders = [
  {
    provide: 'THREAD_REPOSITORY',
    useFactory: async (dataSource: DataSource) =>{
      console.time('threadProviders');
      const {ThreadEntity } = await import('../../entities/thread.entity');
      const result = await dataSource?.getRepository(ThreadEntity)
      console.timeEnd('threadProviders');
      return result;
    },
    inject: ['DATA_SOURCE'],
  },
];

