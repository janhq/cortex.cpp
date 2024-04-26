import { Body, Controller, Post, Res } from '@nestjs/common';
import { CreateChatCompletionDto } from './dto/create-chat-completion.dto';
import { ChatService } from './chat.service';
import { Response } from 'express';

@Controller('chat')
export class ChatController {
  constructor(private readonly chatService: ChatService) {}

  @Post('completions')
  async create(
    @Body() createChatDto: CreateChatCompletionDto,
    @Res() res: Response,
  ) {
    this.chatService.createChatCompletions(createChatDto, res);
  }
}
