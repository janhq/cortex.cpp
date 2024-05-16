import { databaseFile } from 'constant';
import { resolve } from 'path';
import { DataSource } from 'typeorm';

export const sqliteDatabaseProviders = [
  {
    provide: 'DATA_SOURCE',
    useFactory: async () => {
      const sqlitePath = resolve(__dirname, `../../../${databaseFile}`);
      const dataSource = new DataSource({
        type: 'sqlite',
        database: sqlitePath,
        synchronize: process.env.NODE_ENV !== 'production',
        entities: [__dirname + '/../**/*.entity{.ts,.js}'],
      });

      return dataSource.initialize();
    },
  },
];
