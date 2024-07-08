import { TestingModule } from '@nestjs/testing';
import { stubMethod } from 'hanbi';
import { CommandTestFactory } from 'nest-commander-testing';
import { CommandModule } from '@/command.module';
import { join } from 'path';
import { rmSync } from 'fs';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

let commandInstance: TestingModule;

beforeAll(
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
        initialized: false,
        cortexCppHost: 'localhost',
        cortexCppPort: 3929,
      });

      res();
    }),
);

afterAll(
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

export const modelName = 'tinyllama';
describe('Action with models', () => {
  // test('Init with CPU', async () => {
  //   const logMock = stubMethod(console, 'log');
  //
  //   logMock.passThrough();
  //   CommandTestFactory.setAnswers(['CPU', '', 'AVX2']);
  //
  //   await CommandTestFactory.run(commandInstance, ['setup']);
  //   expect(logMock.firstCall?.args[0]).toContain('engine file');
  // }, 50000);
  //

  test('Empty model list', async () => {
    const logMock = stubMethod(console, 'table');

    await CommandTestFactory.run(commandInstance, ['models', 'list']);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Array);
    expect(logMock.firstCall?.args[0].length).toBe(0);
  });

  //
  // test(
  //   'Pull model and check with cortex ps',
  //   async () => {
  //     const logMock = stubMethod(console, 'log');
  //
  //     await CommandTestFactory.run(commandInstance, ['pull', modelName]);
  //     expect(logMock.lastCall?.args[0]).toContain('Download complete!');
  //
  //     const tableMock = stubMethod(console, 'table');
  //     await CommandTestFactory.run(commandInstance, ['ps']);
  //     expect(tableMock.firstCall?.args[0].length).toBeGreaterThan(0);
  //   },
  //   timeout,
  // );
  //
  // test(
  //   'Run model and check with cortex ps',
  //   async () => {
  //     const logMock = stubMethod(console, 'log');
  //
  //     await CommandTestFactory.run(commandInstance, ['run', modelName]);
  //     expect([
  //       "Inorder to exit, type 'exit()'.",
  //       `Model ${modelName} not found. Try pulling model...`,
  //     ]).toContain(logMock.lastCall?.args[0]);
  //
  //     const tableMock = stubMethod(console, 'table');
  //     await CommandTestFactory.run(commandInstance, ['ps']);
  //     expect(tableMock.firstCall?.args[0].length).toBeGreaterThan(0);
  //   },
  //   timeout,
  // );
  //
  // test('Get model', async () => {
  //   const logMock = stubMethod(console, 'log');
  //
  //   await CommandTestFactory.run(commandInstance, ['models', 'get', modelName]);
  //   expect(logMock.firstCall?.args[0]).toBeInstanceOf(Object);
  //   expect(logMock.firstCall?.args[0].files.length).toBe(1);
  // });
  //
  // test('Many models in the list', async () => {
  //   const logMock = stubMethod(console, 'table');
  //   await CommandTestFactory.run(commandInstance, ['models', 'list']);
  //   expect(logMock.firstCall?.args[0]).toBeInstanceOf(Array);
  //   expect(logMock.firstCall?.args[0].length).toBe(1);
  //   expect(logMock.firstCall?.args[0][0].id).toBe(modelName);
  // });
  //
  // test(
  //   'Model already exists',
  //   async () => {
  //     const stdoutSpy = stubMethod(process.stdout, 'write');
  //     const exitSpy = stubMethod(process, 'exit');
  //     await CommandTestFactory.run(commandInstance, ['pull', modelName]);
  //     expect(stdoutSpy.firstCall?.args[0]).toContain('Model already exists');
  //     expect(exitSpy.firstCall?.args[0]).toBe(1);
  //   },
  //   timeout,
  // );
});
