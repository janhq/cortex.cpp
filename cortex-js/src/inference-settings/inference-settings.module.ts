import { Module } from '@nestjs/common';
import { InferenceSettingsService } from './inference-settings.service';
import { InferenceSettingsController } from './inference-settings.controller';
import { DatabaseModule } from 'src/database/database.module';

@Module({
  imports: [DatabaseModule],
  controllers: [InferenceSettingsController],
  providers: [InferenceSettingsService],
  exports: [InferenceSettingsService],
})
export class InferenceSettingsModule {}
