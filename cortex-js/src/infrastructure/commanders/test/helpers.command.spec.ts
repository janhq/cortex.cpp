// import { TestingModule } from '@nestjs/testing';
// import { spy, Stub, stubMethod } from 'hanbi';
// import { CommandTestFactory } from 'nest-commander-testing';
// import { CommandModule } from '@/command.module';
// import { LogService } from '@/infrastructure/commanders/test/log.service';
// import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

// import { join } from 'path';
// import { rmSync } from 'fs';
// import { CortexUsecases } from '@/usecases/cortex/cortex.usecases';

// let commandInstance: TestingModule,
//   exitSpy: Stub<typeof process.exit>,
//   stdoutSpy: Stub<typeof process.stdout.write>,
//   stderrSpy: Stub<typeof process.stderr.write>;
export const timeout = 500000;

// beforeAll(
//   () =>
//     new Promise<void>(async (res) => {
//       stubMethod(process.stderr, 'write');
//       exitSpy = stubMethod(process, 'exit');
//       stdoutSpy = stubMethod(process.stdout, 'write');
//       stderrSpy = stubMethod(process.stderr, 'write');
//       commandInstance = await CommandTestFactory.createTestingCommand({
//         imports: [CommandModule],
//       })
//         .overrideProvider(LogService)
//         .useValue({ log: spy().handler })
//         .compile();

//       const fileService =
//         await commandInstance.resolve<FileManagerService>(FileManagerService);

//       // Attempt to create test folder
//       await fileService.writeConfigFile({
//         dataFolderPath: join(__dirname, 'test_data'),
//         cortexCppHost: 'localhost',
//         cortexCppPort: 3929,
//       });
//       const cortexUseCases =
//         await commandInstance.resolve<CortexUsecases>(CortexUsecases);
//       jest
//         .spyOn(cortexUseCases, 'isAPIServerOnline')
//         .mockImplementation(() => Promise.resolve(true));
//       res();
//     }),
// );

// afterEach(() => {
//   stdoutSpy.reset();
//   stderrSpy.reset();
//   exitSpy.reset();
// });

// afterAll(
//   () =>
//     new Promise<void>(async (res) => {
//       // Attempt to clean test folder
//       rmSync(join(__dirname, 'test_data'), {
//         recursive: true,
//         force: true,
//       });
//       res();
//     }),
// );

describe('Helper commands', () => {
  test(
    'Init with hardware auto detection',
    async () => {
      //   //     await CommandTestFactory.run(commandInstance, ['init', '-s']);
      //   //
      //   //     // Wait for a brief period to allow the command to execute
      //   //     await new Promise((resolve) => setTimeout(resolve, 1000));
      //   //
      //   //     expect(stdoutSpy.firstCall?.args.length).toBeGreaterThan(0);
    },
    timeout,
  );

  //   // test('Chat with option -m', async () => {
  //   //   const logMock = stubMethod(console, 'log');
  //   //
  //   //   await CommandTestFactory.run(commandInstance, [
  //   //     'chat',
  //   //     // '-m',
  //   //     // 'hello',
  //   //     // '>output.txt',
  //   //   ]);
  //   //   expect(logMock.firstCall?.args[0]).toBe("Inorder to exit, type 'exit()'.");
  //   //   // expect(exitSpy.callCount).toBe(1);
  //   //   // expect(exitSpy.firstCall?.args[0]).toBe(1);
});

//   test(
//     'Show stop running models',
//     async () => {
//       // const tableMock = stubMethod(console, 'table');
//       // const logMock = stubMethod(console, 'log');
//       // await CommandTestFactory.run(commandInstance, ['stop']);
//       // await CommandTestFactory.run(commandInstance, ['ps']);
//       // expect(logMock.firstCall?.args[0]).toEqual('API server stopped');
//       // expect(tableMock.firstCall?.args[0]).toBeInstanceOf(Array);
//       // expect(tableMock.firstCall?.args[0].length).toEqual(0);
//     },
//     timeout,
//   );

//   test('Help command return guideline to users', async () => {
//     // await CommandTestFactory.run(commandInstance, ['-h']);
//     // expect(stdoutSpy.firstCall?.args).toBeInstanceOf(Array);
//     // expect(stdoutSpy.firstCall?.args.length).toBe(1);
//     // expect(stdoutSpy.firstCall?.args[0]).toContain('display help for command');
//     // expect(exitSpy.callCount).toBeGreaterThan(1);
//     // expect(exitSpy.firstCall?.args[0]).toBe(0);
//   });

//   test('Should handle missing command', async () => {
//     // await CommandTestFactory.run(commandInstance, ['--unknown']);
//     // expect(stderrSpy.firstCall?.args[0]).toContain('error: unknown option');
//     // expect(stderrSpy.firstCall?.args[0]).toContain('--unknown');
//     // expect(exitSpy.callCount).toBeGreaterThan(0);
//     // expect(exitSpy.firstCall?.args[0]).toBe(1);
//   });

//   // test('Local API server via default host/port localhost:1337/api', async () => {
//   //   await CommandTestFactory.run(commandInstance, ['serve', '--detach']);
//   //
//   //   await new Promise((resolve) => setTimeout(resolve, 2000));
//   //
//   //   expect(stdoutSpy.firstCall?.args[0]).toContain(
//   //     'Started server at http://localhost:1337',
//   //   );
//   //   // Add a delay
//   //   // Temporally disable for further investigation
//   //   return new Promise<void>(async (resolve) => {
//   //     setTimeout(async () => {
//   //       // Send a request to the API server to check if it's running
//   //       const response = await axios.get('http://localhost:1337/api');
//   //       expect(response.status).toBe(200);
//   //       resolve();
//   //     }, 5000);
//   //   });
//   // }, 15000);
// });
