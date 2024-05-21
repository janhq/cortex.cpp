import { Body, Controller, Post, Headers, Res, HttpCode } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { Response } from 'express';
import { ApiResponse, ApiTags } from '@nestjs/swagger';
import { ChatCompletionResponseDto } from '../dtos/chat/chat-completion-response.dto';

@ApiTags('Inference')
@Controller('chat')
export class ChatController {
  constructor(private readonly chatService: ChatUsecases) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Chat completion response successfully',
    type: ChatCompletionResponseDto,
  })
  @Post('completions')
  async create(
    @Headers() headers: Record<string, string>,
    @Body() createChatDto: CreateChatCompletionDto,
    @Res() res: Response,
  ) {
    const { stream } = createChatDto;

    if (stream) {
      this.chatService
        .inferenceStream(createChatDto, headers)
        .then((stream) => stream.pipe(res));
    } else {
      res.json(await this.chatService.inference(createChatDto, headers));
    }
  }
}
