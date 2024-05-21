import { Body, Controller, Post, Headers, Res } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { Response } from 'express';
import {
  ApiCreatedResponse,
  ApiExtraModels,
  ApiOperation,
  ApiTags,
  getSchemaPath,
} from '@nestjs/swagger';
import { ChatStreamEvent } from '@/domain/abstracts/oai.abstract';

@ApiTags('Inference')
@Controller('chat')
export class ChatController {
  constructor(private readonly chatService: ChatUsecases) {}

  @ApiOperation({
    summary: 'Create chat completion',
    description:
      "Creates a model response for the given chat conversation. The API limits stop words to a maximum of 4. If more are specified, only the first four will be accepted. [Equivalent to OpenAI's create chat completion](https://platform.openai.com/docs/api-reference/chat/create).",
  })
  @Post('completions')
  async create(
    @Headers() headers: Record<string, string>,
    @Body() createChatDto: CreateChatCompletionDto,
    @Res() res: Response,
  ) {
    const writableStream = new WritableStream<ChatStreamEvent>({
      write(chunk) {
        if (chunk.type === 'data') {
          res.json(chunk.data ?? {});
        } else if (chunk.type === 'error') {
          res.json(chunk.error ?? {});
        } else {
          console.log('\n');
        }
      },
    });

    this.chatService.createChatCompletions(
      createChatDto,
      headers,
      writableStream,
      res,
    );
  }
}
