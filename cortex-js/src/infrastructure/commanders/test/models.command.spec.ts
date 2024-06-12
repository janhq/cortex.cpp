import { TestingModule } from '@nestjs/testing';
import { stubMethod } from 'hanbi';
import { CommandTestFactory } from 'nest-commander-testing';
import { CommandModule } from '@/command.module';
import { join } from 'path';
import { rmSync } from 'fs';
import { timeout } from '@/infrastructure/commanders/test/helpers.command.spec';

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

export const modelName = 'tinyllama';
describe('Models list returns array of models', () => {
  test('Init with CPU', async () => {
    const logMock = stubMethod(console, 'log');

    logMock.passThrough();
    CommandTestFactory.setAnswers(['CPU', '', 'AVX2']);

    await CommandTestFactory.run(commandInstance, ['init']);
    expect(logMock.firstCall?.args[0]).toBe(
      'Downloading engine file windows-amd64-avx2.tar.gz',
    );
  }, 50000);

  test('Empty model list', async () => {
    const logMock = stubMethod(console, 'table');

    await CommandTestFactory.run(commandInstance, ['models', 'list']);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Array);
    expect(logMock.firstCall?.args[0].length).toBe(0);
  });

  test(
    'Run model and check with cortex ps',
    async () => {
      const logMock = stubMethod(console, 'log');

      await CommandTestFactory.run(commandInstance, ['run', modelName]);
      expect(logMock.lastCall?.args[0]).toBe("Inorder to exit, type 'exit()'.");

      const tableMock = stubMethod(console, 'table');
      await CommandTestFactory.run(commandInstance, ['ps']);
      expect(tableMock.firstCall?.args[0].length).toBeGreaterThan(0);
    },
    timeout,
  );

  test('Get model', async () => {
    const logMock = stubMethod(console, 'log');

    await CommandTestFactory.run(commandInstance, ['models', 'get', modelName]);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Object);
    expect(logMock.firstCall?.args[0].files.length).toBe(1);
  });

  test('Many models in the list', async () => {
    const logMock = stubMethod(console, 'table');
    await CommandTestFactory.run(commandInstance, ['models', 'list']);
    expect(logMock.firstCall?.args[0]).toBeInstanceOf(Array);
    expect(logMock.firstCall?.args[0].length).toBe(1);
    expect(logMock.firstCall?.args[0][0].id).toBe(modelName);
  });

  test(
    'Model already exists',
    async () => {
      const stdoutSpy = stubMethod(process.stdout, 'write');
      const exitSpy = stubMethod(process, 'exit');
      await CommandTestFactory.run(commandInstance, ['pull', modelName]);
      expect(stdoutSpy.firstCall?.args[0]).toContain('Model already exists');
      expect(exitSpy.firstCall?.args[0]).toBe(1);
    },
    timeout,
  );
});
