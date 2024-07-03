#!/usr/bin/env node --no-warnings
console.log('1');
console.time('import');
console.time('imporCommandFactory');
require("time-require");
const { TraceEvents, trackRequires } = require('perftrace')
const { writeFileSync } = require('fs');

const traceEvents = new TraceEvents();

process.on('beforeExit', () => {
  const events = traceEvents.getEvents();
  traceEvents.destroy();
  writeFileSync('events.json', JSON.stringify(events));
});

trackRequires(true);
import { CommandFactory } from 'nest-commander';
console.timeEnd('imporCommandFactory');
console.time('importCommandModule');
import { CommandModule } from './command.module';
console.timeEnd('importCommandModule');
console.time('importTelemetryUsecases');
import { TelemetryUsecases } from './usecases/telemetry/telemetry.usecases';
console.timeEnd('importTelemetryUsecases');
console.time('importContextService');
import { TelemetrySource } from './domain/telemetry/telemetry.interface';
import { ContextService } from '@/infrastructure/services/context/context.service';
console.timeEnd('importContextService');
console.timeEnd('import');
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
