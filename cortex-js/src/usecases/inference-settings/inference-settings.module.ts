import { Module } from '@nestjs/common';
import { InferenceSettingsUsecases } from './inference-settings.usecases';
import { InferenceSettingsController } from '../../infrastructure/controllers/inference-settings.controller';
import { DatabaseModule } from 'src/infrastructure/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [InferenceSettingsController],
  providers: [InferenceSettingsUsecases],
  exports: [InferenceSettingsUsecases],
})
export class InferenceSettingsModule {}
