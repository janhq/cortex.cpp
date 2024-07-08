import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { databaseFile } from '@/infrastructure/constants/cortex';
import { join } from 'path';
import { ThreadEntity } from '../entities/thread.entity';
import { MessageEntity } from '../entities/message.entity';
import { AssistantEntity } from '../entities/assistant.entity';
import { Sequelize } from 'sequelize-typescript';

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
        logging: false,
      });
      sequelize.addModels([ThreadEntity, MessageEntity, AssistantEntity]);
      return sequelize;
    },
  },
];
