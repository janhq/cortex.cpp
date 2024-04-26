import { ThreadEntity } from 'src/threads/entities/thread.entity';
import { DataSource } from 'typeorm';

export const threadProviders = [
  {
    provide: 'THREAD_REPOSITORY',
    useFactory: (dataSource: DataSource) =>
      dataSource.getRepository(ThreadEntity),
    inject: ['DATA_SOURCE'],
  },
];
