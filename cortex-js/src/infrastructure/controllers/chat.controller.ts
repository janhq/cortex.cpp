import { Body, Controller, Post, Res } from '@nestjs/common';
import { CreateChatCompletionDto } from '../dtos/chat/create-chat-completion.dto';
import { ChatUsecases } from '../../usecases/chat/chat.usecases';
import { Response } from 'express';
import { ApiTags } from '@nestjs/swagger';

@ApiTags('Inference')
@Controller('chat')
export class ChatController {
  constructor(private readonly chatService: ChatUsecases) {}

  @Post('completions')
  async create(
    @Body() createChatDto: CreateChatCompletionDto,
    @Res() res: Response,
  ) {
    return this.chatService.createChatCompletions(createChatDto, res);
  }
}
