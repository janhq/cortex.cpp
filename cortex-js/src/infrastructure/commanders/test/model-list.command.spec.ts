import { TestingModule } from '@nestjs/testing';
import { stubMethod } from 'hanbi';
import { CommandTestFactory } from 'nest-commander-testing';
import { CommandModule } from '@/command.module';
import { FileManagerService } from '@/file-manager/file-manager.service';
import { join } from 'path';
import { mkdirSync, rmSync, writeFileSync } from 'fs';
import * as process from 'node:process';
import axios from 'axios';

let commandInstance: TestingModule;

beforeEach(
  () =>
    new Promise<void>(async (res) => {
      commandInstance = await CommandTestFactory.createTestingCommand({
        imports: [CommandModule],
      })
        // .overrideProvider(LogService)
        // .useValue({})
        .compile();
      const fileService =
        await commandInstance.resolve<FileManagerService>(FileManagerService);

      // Attempt to create test folder
      await fileService.writeConfigFile({
        dataFolderPath: join(__dirname, 'test_data'),
      });
      res();
    }),
);

afterEach(
  () =>
    new Promise<void>(async (res) => {
      // Attempt to clean test folder
      rmSync(join(__dirname, 'test_data'), {
        recursive: true,
        force: true,
      });
      res();
    }),
);

const llama3 = 'llama3';
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

  test('pull existing model', async () => {
    const logMock = stubMethod(console, 'error');

    await CommandTestFactory.run(commandInstance, ['pull', llama3]);
    expect(logMock.firstCall?.args[0]).toBe('Model already exists');
  });

  test('run model', async () => {
    const logMock = stubMethod(console, 'log');

    await CommandTestFactory.run(commandInstance, ['run', llama3]);
    expect(logMock.firstCall?.args[0]).toBe("Inorder to exit, type 'exit()'.");

    const tableMock = stubMethod(console, 'table');
    await CommandTestFactory.run(commandInstance, ['ps']);
    expect(tableMock.firstCall?.args[0].length).toBeGreaterThan(0);
  });

  test('get model', async () => {
    const logMock = stubMethod(console, 'log');

    await CommandTestFactory.run(commandInstance, ['models', 'get', llama3]);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Object);
    expect(logMock.firstCall?.args[0].files.length).toBe(1);
  });

  test('hello world', async () => {
    const logMock = stubMethod(process.stdout, 'write');

    await CommandTestFactory.run(commandInstance, ['chat', '-m', 'hello']);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Array);
  });

  test('local API server via localhost:1337/api', async () => {
    await CommandTestFactory.run(commandInstance, ['serve']);

    // Send a request to the API server to check if it's running
    const response = await axios.get('http://localhost:1337/api');
    expect(response.status).toBe(200);
  });
});
