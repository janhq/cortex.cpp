export const assistantProviders = [
  {
    provide: 'ASSISTANT_REPOSITORY',
    useFactory: async(dataSource: any) =>{
      const {AssistantEntity } = await import('../../entities/assistant.entity');
      const result = await dataSource?.getRepository(AssistantEntity)
      return result;
    },
    inject: ['DATA_SOURCE'],
  },
];
