import { MessageEntity } from 'src/messages/entities/message.entity';
import { DataSource } from 'typeorm';

export const messageProviders = [
  {
    provide: 'MESSAGE_REPOSITORY',
    useFactory: (dataSource: DataSource) =>
      dataSource.getRepository(MessageEntity),
    inject: ['DATA_SOURCE'],
  },
];
