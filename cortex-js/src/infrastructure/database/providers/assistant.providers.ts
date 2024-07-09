import { AssistantEntity } from '@/infrastructure/entities/assistant.entity';
import { Sequelize } from 'sequelize-typescript';

export const assistantProviders = [
  {
    provide: 'ASSISTANT_REPOSITORY',
    useFactory: async(sequelize: Sequelize) =>{
      return sequelize.getRepository(AssistantEntity);
    },
    inject: ['DATA_SOURCE'],
  },
];
