import { databaseFile } from '@/infrastructure/constants/cortex';
import { join } from 'path';
import { ThreadEntity } from '../entities/thread.entity';
import { MessageEntity } from '../entities/message.entity';
import { AssistantEntity } from '../entities/assistant.entity';
import { Sequelize } from 'sequelize-typescript';
import { fileManagerService } from '../services/file-manager/file-manager.service';
import { VectorStoreEntity } from '../entities/vector_store.entity';

export const sqliteDatabaseProviders = [
  {
    provide: 'DATA_SOURCE',
    inject: [],
    useFactory: async () => {
      const dataFolderPath = await fileManagerService.getDataFolderPath();
      const sqlitePath = join(dataFolderPath, databaseFile);
      const sequelize = new Sequelize({
        dialect: 'sqlite',
        storage: sqlitePath,
        logging: false,
      });
      sequelize.addModels([
        ThreadEntity,
        MessageEntity,
        AssistantEntity,
        VectorStoreEntity,
      ]);
      await ThreadEntity.sync();
      await MessageEntity.sync();
      await AssistantEntity.sync();
      await VectorStoreEntity.sync();
      return sequelize;
    },
  },
];
