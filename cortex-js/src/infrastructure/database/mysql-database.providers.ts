import { databaseName } from '@/infrastructure/constants/cortex';
import { ThreadEntity } from '../entities/thread.entity';
import { AssistantEntity } from '../entities/assistant.entity';
import { MessageEntity } from '../entities/message.entity';

export const mysqlDatabaseProviders = [
  {
    provide: 'DATA_SOURCE',
    useFactory: async () => {
      const {DataSource} = await import('typeorm');
      const dataSource = new DataSource({
        type: 'mysql',
        host: 'localhost',
        port: 3306,
        username: 'root',
        password: '',
        database: databaseName,
        entities: [ThreadEntity, AssistantEntity, MessageEntity],
        synchronize: process.env.NODE_ENV !== 'production',
      });

      return dataSource.initialize();
    },
  },
];
