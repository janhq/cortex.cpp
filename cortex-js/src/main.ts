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
    .setDescription('Cortex API is compatible with the [OpenAI API](https://platform.openai.com/docs/api-reference)')
    .setVersion('1.0')
    .addTag('Cortex', 'These endpoints control the start and stop operations of the Cortex system.')
    .addTag('Inference', 'This endpoint initiates interaction with a Language Learning Model (LLM).')
    .addTag('Assistants', 'These endpoints manage the lifecycle of an Assistant within a conversation thread.')
    .addTag('Models', 'These endpoints provide a list and descriptions of all available models within the Cortex framework.')
    .addTag('Messages', "These endpoints manage the retrieval and storage of conversation content, including responses from LLMs and other metadata related to chat interactions.")
    .addTag('Threads', 'These endpoints handle the creation, retrieval, updating, and deletion of conversation threads.')
    .addServer('http://localhost:7331/')
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
