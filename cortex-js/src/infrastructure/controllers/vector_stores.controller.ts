import {
  Body,
  Controller,
  Post,
  HttpCode,
  Get,
  Param,
  Delete,
} from '@nestjs/common';
import { ApiOperation, ApiTags, ApiResponse, ApiParam } from '@nestjs/swagger';
import { CreateVectorStoresDto } from '../dtos/vector_stores/create.vector_stores';
import { VectorStoresUsecases } from '@/usecases/vector_stores/vector_stores.usecases';
import { CommonResponseDto } from '../dtos/common/common-response.dto';
import { VectorStoreDto } from '../dtos/vector_stores/vector-store';

@ApiTags('Vector Stores')
@Controller('vector_stores')
export class VectorStoresController {
  constructor(private readonly usecases: VectorStoresUsecases) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: [VectorStoreDto],
  })
  @ApiOperation({
    summary: 'List created vector stores',
    description: 'Lists created vector stores.',
  })
  @Get()
  findAll() {
    return this.usecases.getAll();
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: VectorStoreDto,
  })
  @ApiOperation({
    summary: 'Get a vector store',
    description: 'Retrieves a created vector store instance',
  })
  @ApiParam({
    name: 'name',
    required: true,
    description: 'The unique identifier of the vector store.',
  })
  @Get(':name(*)')
  findOne(@Param('name') name: string) {
    return this.usecases.get(name);
  }

  @ApiOperation({
    summary: 'Create a vector store.',
    description:
      'Vector stores are used to store files for use by the file_search tool.',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CreateVectorStoresDto,
  })
  @Post()
  async create(@Body() createVectorStore: CreateVectorStoresDto) {
    return this.usecases.create(createVectorStore);
  }

  @ApiOperation({
    summary: 'Delete a vector store.',
    description: 'Delete a created vector store.',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto,
  })
  @Delete()
  async delete(@Param('name') name: string) {
    return this.usecases.remove(name);
  }
}
