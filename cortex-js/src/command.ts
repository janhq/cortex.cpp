import { CommandFactory } from 'nest-commander';
import { CommandModule } from './command.module';

async function bootstrap() {
  await CommandFactory.run(CommandModule);
}

bootstrap();
