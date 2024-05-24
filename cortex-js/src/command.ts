#!/usr/bin/env node --no-warnings
import { CommandFactory } from 'nest-commander';
import { CommandModule } from './command.module';

async function bootstrap() {
  await CommandFactory.run(CommandModule, ['warn', 'error']);
}

bootstrap();
