import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { DocumentBuilder, SwaggerModule } from '@nestjs/swagger';
import { ValidationPipe } from '@nestjs/common';
import { defaultCortexJsHost, defaultCortexJsPort } from 'constant';

async function bootstrap() {
  const app = await NestFactory.create(AppModule, {
    snapshot: true,
    cors: true,
  });

  app.useGlobalPipes(
    new ValidationPipe({
      enableDebugMessages: true,
    }),
  );

  const config = new DocumentBuilder()
    .setTitle('Cortex API')
    .setDescription('The Cortex API description')
    .setVersion('1.0')
    .build();
  const document = SwaggerModule.createDocument(app, config);

  SwaggerModule.setup('api', app, document);

  // getting port from env
  const host = process.env.CORTEX_JS_HOST || defaultCortexJsHost;
  const port = process.env.CORTEX_JS_PORT || defaultCortexJsPort;

  await app.listen(port, host);
  console.log(`Server running on ${host}:${port}`);
}

bootstrap();
