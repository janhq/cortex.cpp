import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { DocumentBuilder, SwaggerModule } from '@nestjs/swagger';
import { INestApplication, ValidationPipe } from '@nestjs/common';
import { defaultCortexJsHost, defaultCortexJsPort } from 'constant';
import { SeedService } from './usecases/seed/seed.service';
import { FileManagerService } from './file-manager/file-manager.service';

async function bootstrap() {
  const app = await NestFactory.create(AppModule, {
    snapshot: true,
    cors: true,
  });
  const seedService = app.get(SeedService);
  await seedService.seed();

  const fileService = app.get(FileManagerService);
  await fileService.getConfig();

  app.useGlobalPipes(
    new ValidationPipe({
      enableDebugMessages: true,
    }),
  );

  buildSwagger(app);

  // getting port from env
  const host = process.env.CORTEX_JS_HOST || defaultCortexJsHost;
  const port = process.env.CORTEX_JS_PORT || defaultCortexJsPort;

  await app.listen(port, host);
  console.log(`Server running on ${host}:${port}`);
}

const buildSwagger = (app: INestApplication<any>) => {
  const config = new DocumentBuilder()
    .setTitle('Cortex API')
    .setDescription('The Cortex API description')
    .setVersion('1.0')
    .build();
  const document = SwaggerModule.createDocument(app, config);

  SwaggerModule.setup('api', app, document);
};

bootstrap();
