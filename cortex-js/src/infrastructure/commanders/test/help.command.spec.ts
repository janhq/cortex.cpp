import { TestingModule } from '@nestjs/testing';
import { spy, Stub, stubMethod } from 'hanbi';
import { CommandTestFactory } from 'nest-commander-testing';
import { CommandModule } from '@/command.module';
import { LogService } from '@/infrastructure/commanders/test/log.service';

let commandInstance: TestingModule,
  exitSpy: Stub<typeof process.exit>,
  stdoutSpy: Stub<typeof process.stdout.write>;

beforeEach(
  () =>
    new Promise<void>(async (res) => {
      stubMethod(process.stderr, 'write');
      exitSpy = stubMethod(process, 'exit');
      stdoutSpy = stubMethod(process.stdout, 'write');
      commandInstance = await CommandTestFactory.createTestingCommand({
        imports: [CommandModule],
      })
        .overrideProvider(LogService)
        .useValue({ log: spy().handler })
        .compile();
      res();
      exitSpy.reset();
      stdoutSpy.reset();
    }),
);

describe('Help command return guideline to users ', () => {
  test('should return guideline', async () => {
    const stdoutSpy = stubMethod(process.stdout, 'write');

    await CommandTestFactory.run(commandInstance, ['-h']);
    expect(stdoutSpy.firstCall?.args).toBeInstanceOf(Array);
    expect(stdoutSpy.firstCall?.args[0]).toContain('display help for command');
    expect(stdoutSpy.firstCall?.args.length).toBe(1);
  });
});
