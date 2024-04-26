import { AssistantEntity } from 'src/assistants/entities/assistant.entity';
import { DataSource } from 'typeorm';

export const assistantProviders = [
  {
    provide: 'ASSISTANT_REPOSITORY',
    useFactory: (dataSource: DataSource) =>
      dataSource.getRepository(AssistantEntity),
    inject: ['DATA_SOURCE'],
  },
];
