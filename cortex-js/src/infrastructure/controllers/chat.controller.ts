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
    this.chatService
      .inference(createChatDto, extractCommonHeaders(headers))
      .then((response) => {
        if (stream) {
          res.header('Content-Type', 'text/event-stream');
          response.pipe(res);
        } else {
          res.header('Content-Type', 'application/json');
          res.json(response);
        }
      })
      .catch((error) => {
        const statusCode = error.response?.status ?? 400;
        let errorMessage;
        if (!stream) {
          const data = error.response?.data;
          return res
            .status(statusCode)
            .send(
              data.error?.message ||
                data.message ||
                error.message ||
                'An error occurred',
            );
        }
        const streamResponse = error.response?.data;
        let data = '';

        streamResponse.on('data', (chunk: any) => {
          data += chunk;
        });

        streamResponse.on('end', () => {
          try {
            const jsonResponse = JSON.parse(data);
            errorMessage =
              jsonResponse.error?.message ||
              jsonResponse.message ||
              error.message ||
              'An error occurred';
          } catch (err) {
            errorMessage = 'An error occurred while processing the response';
          }
          return res
            .status(error.statusCode ?? 400)
            .send(errorMessage || error.message || 'An error occurred');
        });

        streamResponse.on('error', (streamError: any) => {
          errorMessage = streamError.message ?? 'An error occurred';
          return res
            .status(error.statusCode ?? 400)
            .send(errorMessage || error.message || 'An error occurred');
        });
      });

    this.telemetryUsecases.addEventToQueue({
      name: EventName.CHAT,
      modelId: createChatDto.model,
    });
  }
}
