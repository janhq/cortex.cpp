export const messageProviders = [
  {
    provide: 'MESSAGE_REPOSITORY',
    useFactory: async(dataSource: any) =>{
      const {MessageEntity } = await import('../../entities/message.entity');
      const result = await dataSource?.getRepository(MessageEntity)
      return result;
    },
    inject: ['DATA_SOURCE'],
  },
];
