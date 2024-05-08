import { NestFactory } from '@nestjs/core';
import { AppModule } from './app.module';
import { DocumentBuilder, SwaggerModule } from '@nestjs/swagger';
import { ValidationPipe } from '@nestjs/common';

async function bootstrap() {
  const app = await NestFactory.create(AppModule, {
    snapshot: true,
    cors: true,
  });

  app.useGlobalPipes(
    new ValidationPipe({
      enableDebugMessages: true,
      whitelist: true,
    }),
  );

  const config = new DocumentBuilder()
    .setTitle('Cortex API')
    .setDescription('The Cortex API description')
    .setVersion('1.0')
    .build();
  const document = SwaggerModule.createDocument(app, config);

  SwaggerModule.setup('api', app, document);

  await app.listen(7331);
}
bootstrap();
