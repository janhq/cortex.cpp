import { Body, Controller, Post, HttpCode } from '@nestjs/common';
import { ChatUsecases } from '@/usecases/chat/chat.usecases';
import { ApiOperation, ApiTags, ApiResponse } from '@nestjs/swagger';
import { CreateEmbeddingsDto } from '../dtos/embeddings/embeddings-request.dto';
import { EmbeddingsResponseDto } from '../dtos/chat/embeddings-response.dto';

@ApiTags('Embeddings')
@Controller('embeddings')
export class EmbeddingsController {
  constructor(private readonly chatService: ChatUsecases) {}

  @ApiOperation({
    summary: 'Embedding vector for text',
    description: 'Creates an embedding vector representing the input text..',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: EmbeddingsResponseDto,
  })
  @Post()
  async create(@Body() createEmbeddingsDto: CreateEmbeddingsDto) {
    return this.chatService.embeddings(createEmbeddingsDto);
  }
}
