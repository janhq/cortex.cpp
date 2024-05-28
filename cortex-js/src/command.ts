#!/usr/bin/env node --no-warnings
import { CommandFactory } from 'nest-commander';
import { CommandModule } from './command.module';
import updateNotifier from 'update-notifier';
import packageJson from './../package.json';

async function bootstrap() {
  await CommandFactory.run(CommandModule, ['warn', 'error']);
  const notifier = updateNotifier({
    pkg: packageJson,
    updateCheckInterval: 1000 * 60 * 60, // 1 hour
    shouldNotifyInNpmScript: true,
  });
  notifier.notify({
    isGlobal: true,
  });
}

bootstrap();
