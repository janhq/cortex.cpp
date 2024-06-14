import { Module } from '@nestjs/common';
import { ContextService } from './context.service';

@Module({
  //move context service to usecase?
  providers: [ContextService],
  exports: [ContextService],
})
export class UtilModule {}
