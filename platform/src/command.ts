#!/usr/bin/env node
import ora from 'ora';
const dependenciesSpinner = ora('Loading dependencies...').start();
const time = Date.now();
import { CommandFactory } from 'nest-commander';
import { CommandModule } from './command.module';
import { TelemetryUsecases } from './usecases/telemetry/telemetry.usecases';
import { TelemetrySource } from './domain/telemetry/telemetry.interface';
import { ContextService } from '@/infrastructure/services/context/context.service';

dependenciesSpinner.succeed(
  'Dependencies loaded in ' + (Date.now() - time) + 'ms',
);

process.removeAllListeners('warning');
process.title = 'Cortex CLI Command Process';

async function bootstrap() {
  let telemetryUseCase: TelemetryUsecases | null = null;
  let contextService: ContextService | null = null;
  const app = await CommandFactory.createWithoutRunning(CommandModule, {
    logger: ['warn', 'error'],
    enablePositionalOptions: true,
    errorHandler: async (error) => {
      await telemetryUseCase!.createCrashReport(error, TelemetrySource.CLI);
      console.error(error);
      process.exit(1);
    },
    serviceErrorHandler: async (error) => {
      await telemetryUseCase!.createCrashReport(error, TelemetrySource.CLI);
      console.error(error);
      process.exit(1);
    },
  });

  telemetryUseCase = await app.resolve(TelemetryUsecases);
  contextService = await app.resolve(ContextService);

  const anonymousData = await telemetryUseCase!.updateAnonymousData();

  await contextService!.init(async () => {
    contextService!.set('source', TelemetrySource.CLI);
    contextService!.set('sessionId', anonymousData?.sessionId);
    telemetryUseCase!.sendActivationEvent(TelemetrySource.CLI);
    telemetryUseCase!.sendCrashReport();
    await CommandFactory.runApplication(app);
  });
}

bootstrap();
