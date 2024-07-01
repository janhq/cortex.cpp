#!/usr/bin/env node --no-warnings
import { CommandFactory } from 'nest-commander';
import { CommandModule } from './command.module';
import { TelemetryUsecases } from './usecases/telemetry/telemetry.usecases';
import { TelemetrySource } from './domain/telemetry/telemetry.interface';
// import { AsyncLocalStorage } from 'async_hooks';
import { ContextService } from '@/infrastructure/services/context/context.service';

// export const asyncLocalStorage = new AsyncLocalStorage();
console.time('test');
console.log('Running CLI');
async function bootstrap() {
  let telemetryUseCase: TelemetryUsecases | null = null;
  let contextService: ContextService | null = null;
  console.time('CLI');
  const app = await CommandFactory.createWithoutRunning(CommandModule, {
    logger: ['warn', 'error'],
    errorHandler: async (error) => {
      await telemetryUseCase!.createCrashReport(error, TelemetrySource.CLI);
      process.exit(1);
    },
    serviceErrorHandler: async (error) => {
      await telemetryUseCase!.createCrashReport(error, TelemetrySource.CLI);
      process.exit(1);
    },
  });

  telemetryUseCase = await app.resolve(TelemetryUsecases);
  contextService = await app.resolve(ContextService);

  telemetryUseCase!.sendCrashReport();

  await contextService!.init(async () => {
    contextService!.set('source', TelemetrySource.CLI);
    await CommandFactory.runApplication(app);
    // await CommandFactory.run(CommandModule, ['warn', 'error']);
    console.timeEnd('CLI');
    console.timeEnd('test');
  });
}

bootstrap();
