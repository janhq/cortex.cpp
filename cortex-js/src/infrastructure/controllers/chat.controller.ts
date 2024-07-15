import { Body, Controller, Post, Headers, Res, HttpCode } from '@nestjs/common';
import { CreateChatCompletionDto } from '@/infrastructure/dtos/chat/create-chat-completion.dto';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { Response } from 'express';
import { ApiOperation, ApiTags, ApiResponse } from '@nestjs/swagger';
import { ChatCompletionResponseDto } from '../dtos/chat/chat-completion-response.dto';
import { TelemetryUsecases } from '@/usecases/telemetry/telemetry.usecases';
import { EventName } from '@/domain/telemetry/telemetry.interface';
import { extractCommonHeaders } from '@/utils/request';

@ApiTags('Inference')
@Controller('chat')
export class ChatController {
  constructor(
    private readonly chatService: ChatUsecases,
    private readonly telemetryUsecases: TelemetryUsecases,
  ) {}

  @ApiOperation({
    summary: 'Create chat completion',
    description:
      'Creates a model response for the given conversation. The following parameters are not working for the `TensorRT-LLM` engine:\n- `frequency_penalty`\n- `presence_penalty`\n- `top_p`',
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
      this.chatService
        .inference(createChatDto, extractCommonHeaders(headers))
        .then((stream) => {
          res.header('Content-Type', 'text/event-stream');
          stream.pipe(res);
        })
        .catch((error) =>
          res.status(error.statusCode ?? 400).send(error.message),
        );
    } else {
      res.header('Content-Type', 'application/json');
      this.chatService
        .inference(createChatDto, extractCommonHeaders(headers))
        .then((response) => {
          res.json(response);
        })
        .catch((error) =>
          res.status(error.statusCode ?? 400).send(error.message),
        );
    }
    this.telemetryUsecases.addEventToQueue({
      name: EventName.CHAT,
      modelId: createChatDto.model,
    });
  }
}
