import { Test, TestingModule } from '@nestjs/testing';
import { AssistantsController } from './assistants.controller';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { DatabaseModule } from '@/infrastructure/database/database.module';
import { ModelRepositoryModule } from '../repositories/models/model.module';
import { DownloadManagerModule } from '../services/download-manager/download-manager.module';
import { EventEmitterModule } from '@nestjs/event-emitter';
import { faker } from '@faker-js/faker';
import { CreateAssistantDto } from '../dtos/assistants/create-assistant.dto';
import { ModelRepository } from '@/domain/repositories/model.interface';

describe('AssistantsController', () => {
  let module: TestingModule;
  let controller: AssistantsController;

  const testAssistant: CreateAssistantDto = {
    id: faker.string.uuid(),
    name: faker.string.alpha(),
    description: faker.string.alpha(),
    instructions: faker.string.alpha(),
    metadata: faker.string.alpha(),
    model: 'llama3:main',
    tools: [],
  };

  beforeAll(async () => {
    module = await Test.createTestingModule({
      imports: [
        EventEmitterModule.forRoot(),
        DatabaseModule,
        ModelRepositoryModule,
        DownloadManagerModule,
      ],
      controllers: [AssistantsController],
      providers: [AssistantsUsecases],
      exports: [AssistantsUsecases],
    })
      .overrideProvider(AssistantsUsecases)
      .useValue({
        create: jest.fn().mockResolvedValue(testAssistant),
        listAssistants: jest.fn().mockResolvedValue([testAssistant]),
        findOne: jest.fn().mockResolvedValue(testAssistant),
      })
      .compile();

    controller = module.get<AssistantsController>(AssistantsController);
  });

  afterAll(async () => {});

  it('should be defined', () => {
    expect(controller).toBeDefined();
  });

  it('create assistant', async () => {
    expect(controller.create(testAssistant)).resolves.toHaveProperty(
      'name',
      testAssistant.name,
    );
  });

  it('get assistants', () => {
    expect(controller.findAll(10, 'asc')).resolves.toHaveLength(1);
  });

  it('get assistant', () => {
    expect(controller.findOne(testAssistant.id)).resolves.toHaveProperty(
      'name',
      testAssistant.name,
    );
  });
});
