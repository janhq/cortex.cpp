import { databaseFile } from 'constant';
import { DataSource } from 'typeorm';

export const sqliteDatabaseProviders = [
  {
    provide: 'DATA_SOURCE',
    useFactory: async () => {
      const dataSource = new DataSource({
        type: 'sqlite',
        database: databaseFile,
        synchronize: process.env.NODE_ENV !== 'production',
        entities: [__dirname + '/../**/*.entity{.ts,.js}'],
      });

      return dataSource.initialize();
    },
  },
];
