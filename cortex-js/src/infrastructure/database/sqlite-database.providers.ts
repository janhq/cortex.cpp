console.time('sqliteDatabaseProviders-import');
console.time('sqliteDatabaseProviders-file-manager-service');
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
console.timeEnd('sqliteDatabaseProviders-file-manager-service');
console.timeEnd('sqliteDatabaseProviders-database-file');
import { databaseFile } from '@/infrastructure/constants/cortex';
console.timeEnd('sqliteDatabaseProviders-import');
console.time('sqliteDatabaseProviders-import-path');
import { join } from 'path';
console.timeEnd('sqliteDatabaseProviders-import-path');
console.time('sqliteDatabaseProviders-import-typeorm');
import { DataSource } from 'typeorm';
console.timeEnd('sqliteDatabaseProviders-import-typeorm');
console.time('sqliteDatabaseProviders-import');

export const sqliteDatabaseProviders = [
  {
    provide: 'DATA_SOURCE',
    inject: [FileManagerService],
    useFactory: async (fileManagerService: FileManagerService) => {
      console.time('sqliteDatabaseProviders');
      const dataFolderPath = await fileManagerService.getDataFolderPath();
      const { ThreadEntity } = await import('../entities/thread.entity');
      const { AssistantEntity } = await import('../entities/assistant.entity');
      const { MessageEntity } = await import('../entities/message.entity');
      const sqlitePath = join(dataFolderPath, databaseFile);
      
      const dataSource = new DataSource({
        type: 'sqlite',
        database: sqlitePath,
        synchronize: false,
        entities: [ThreadEntity, AssistantEntity, MessageEntity],
        logging: true, 
      });
      const result = await dataSource.initialize();
      console.timeEnd('sqliteDatabaseProviders');
      return result;
    },
  },
];
