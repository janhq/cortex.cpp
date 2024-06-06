import { TestingModule } from '@nestjs/testing';
import { stubMethod } from 'hanbi';
import { CommandTestFactory } from 'nest-commander-testing';
import { CommandModule } from '@/command.module';
import { FileManagerService } from '@/file-manager/file-manager.service';
import { join } from 'path';
import { mkdirSync, rmSync, writeFileSync } from 'fs';

let commandInstance: TestingModule;

beforeEach(async () => {
  commandInstance = await CommandTestFactory.createTestingCommand({
    imports: [CommandModule],
  })
    // .overrideProvider(LogService)
    // .useValue({})
    .compile();
  const fileService =
    commandInstance.resolve<FileManagerService>(FileManagerService);

  // Attempt to create test folder
  (await fileService).writeConfigFile({
    dataFolderPath: join(__dirname, 'test_data'),
  });
});

afterEach(async () => {
  // Attempt to clean test folder
  try {
    await rmSync(join(__dirname, 'test_data'), {
      recursive: true,
      force: true,
    });
  } catch (e) {}
});

describe('models list returns array of models', () => {
  test('empty model list', async () => {
    const logMock = stubMethod(console, 'table');

    await CommandTestFactory.run(commandInstance, ['models', 'list']);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Array);
    expect(logMock.firstCall?.args[0].length).toBe(0);
  });

  test('many models in the list', async () => {
    const logMock = stubMethod(console, 'table');

    mkdirSync(join(__dirname, 'test_data', 'models'), { recursive: true });
    writeFileSync(
      join(__dirname, 'test_data', 'models', 'test.yaml'),
      'model: test',
      'utf8',
    );

    await CommandTestFactory.run(commandInstance, ['models', 'list']);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Array);
    expect(logMock.firstCall?.args[0].length).toBe(1);
    expect(logMock.firstCall?.args[0][0].id).toBe('test');
  });
});
