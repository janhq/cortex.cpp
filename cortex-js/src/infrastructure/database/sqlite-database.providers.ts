import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { databaseFile } from '@/infrastructure/constants/cortex';
import { join } from 'path';
import { DataSource } from 'typeorm';
import { ThreadEntity } from '../entities/thread.entity';
import { AssistantEntity } from '../entities/assistant.entity';
import { MessageEntity } from '../entities/message.entity';

export const sqliteDatabaseProviders = [
  {
    provide: 'DATA_SOURCE',
    inject: [FileManagerService],
    useFactory: async (fileManagerService: FileManagerService) => {
      const dataFolderPath = await fileManagerService.getDataFolderPath();
      const sqlitePath = join(dataFolderPath, databaseFile);
      const dataSource = new DataSource({
        type: 'sqlite',
        database: sqlitePath,
        synchronize: process.env.NODE_ENV !== 'production',
        entities: [ThreadEntity, AssistantEntity, MessageEntity],
      });

      return dataSource.initialize();
    },
  },
];
