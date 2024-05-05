import { Module } from '@nestjs/common';
import { ExtensionRepositoryImpl } from './extension.repository';
import { ExtensionRepository } from 'src/domain/repositories/extension.interface';

@Module({
  providers: [
    {
      provide: ExtensionRepository,
      useClass: ExtensionRepositoryImpl,
    },
  ],
  exports: [ExtensionRepository],
})
export class ExtensionModule {}
