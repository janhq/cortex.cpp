import { Body, Controller, Post, Headers, Res } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { Response } from 'express';
import { ApiTags } from '@nestjs/swagger';
import { ChatStreamEvent } from '@/domain/abstracts/oai.abstract';

@ApiTags('Inference')
@Controller('chat')
export class ChatController {
  constructor(private readonly chatService: ChatUsecases) {}

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
