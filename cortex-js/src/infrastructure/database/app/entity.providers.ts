import { AssistantEntity } from '@/infrastructure/entities/assistant.entity';
import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { ThreadEntity } from '@/infrastructure/entities/thread.entity';
import { VectorStoreEntity } from '@/infrastructure/entities/vector_store.entity';

import { Sequelize } from 'sequelize-typescript';

export const entityProviders = [
  {
    provide: 'ASSISTANT_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(AssistantEntity);
    },
    inject: ['DATA_SOURCE'],
  },
  {
    provide: 'MESSAGE_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(MessageEntity);
    },
    inject: ['DATA_SOURCE'],
  },
  {
    provide: 'THREAD_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(ThreadEntity);
    },
    inject: ['DATA_SOURCE'],
  },
  {
    provide: 'VECTOR_STORE_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(VectorStoreEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];
