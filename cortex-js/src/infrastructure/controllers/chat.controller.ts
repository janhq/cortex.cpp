import { Body, Controller, Post, Headers, Res, HttpCode } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { Response } from 'express';
import { ApiOperation, ApiTags, ApiResponse } from '@nestjs/swagger';
import { ChatCompletionResponseDto } from '../dtos/chat/chat-completion-response.dto';

@ApiTags('Inference')
@Controller('chat')
export class ChatController {
  constructor(private readonly chatService: ChatUsecases) {}

  @ApiOperation({
    summary: 'Create chat completion',
    description: 'Creates a model response for the given conversation. The following parameters are not working for the `TensorRT-LLM` engine:\n- `frequency_penalty`\n- `presence_penalty`\n- `top_p`',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
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
      res.header('Content-Type', 'text/event-stream');
      this.chatService
        .inference(createChatDto, headers)
        .then((stream) => stream.pipe(res));
    } else {
      res.header('Content-Type', 'application/json');
      res.json(await this.chatService.inference(createChatDto, headers));
    }
  }
}
