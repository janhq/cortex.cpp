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
import { ThreadEntity } from '../entities/thread.entity';
import { MessageEntity } from '../entities/message.entity';
import { AssistantEntity } from '../entities/assistant.entity';
import { Sequelize } from 'sequelize-typescript';
console.timeEnd('sqliteDatabaseProviders-import-typeorm');
console.time('sqliteDatabaseProviders-import');

export const sqliteDatabaseProviders = [
  {
    provide: 'DATA_SOURCE',
    inject: [FileManagerService],
    useFactory: async (fileManagerService: FileManagerService) => {
      const dataFolderPath = await fileManagerService.getDataFolderPath();
      const sqlitePath = join(dataFolderPath, databaseFile);
      
      const sequelize = new Sequelize({
        dialect: 'sqlite',
        storage: sqlitePath,
      });
      sequelize.addModels([ThreadEntity, MessageEntity, AssistantEntity]);
      return sequelize;
    },
  },
];
