import { Test, TestingModule } from '@nestjs/testing';
import { ChatUsecases } from './chat.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ExtensionModule } from '@/infrastructure/repositories/extensions/extension.module';
import { TelemetryModule } from '../telemetry/telemetry.module';

describe('ChatService', () => {
  let service: ChatUsecases;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      imports: [DatabaseModule, ExtensionModule, TelemetryModule],
      providers: [ChatUsecases],
      exports: [ChatUsecases],
    }).compile();

    service = module.get<ChatUsecases>(ChatUsecases);
  });

  it('should be defined', () => {
    expect(service).toBeDefined();
  });
});
