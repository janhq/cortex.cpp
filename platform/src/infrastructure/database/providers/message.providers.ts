import { MessageEntity } from '@/infrastructure/entities/message.entity';
import { Sequelize } from 'sequelize-typescript';

export const messageProviders = [
  {
    provide: 'MESSAGE_REPOSITORY',
    useFactory: async (sequelize: Sequelize) => {
      return sequelize.getRepository(MessageEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];
