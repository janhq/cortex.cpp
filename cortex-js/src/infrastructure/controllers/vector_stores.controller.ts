import {
  Body,
  Controller,
  Post,
  HttpCode,
  Get,
  Param,
  Delete,
  Patch,
} from '@nestjs/common';
import { ApiOperation, ApiTags, ApiResponse, ApiParam } from '@nestjs/swagger';
import { CreateVectorStoresDto } from '../dtos/vector_stores/create.vector_stores';
import { VectorStoresUsecases } from '@/usecases/vector_stores/vector_stores.usecases';
import { CommonResponseDto } from '../dtos/common/common-response.dto';
import { VectorStoreDto } from '../dtos/vector_stores/vector-store';
import { HttpStatusCode } from 'axios';

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
    summary: 'List vector stores',
    description: 'Returns a list of vector stores.',
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
    summary: 'Retrieve vector store',
    description: 'Retrieves a vector store.',
  })
  @ApiParam({
    name: 'name',
    required: true,
    description: 'The unique identifier of the vector store.',
  })
  @Get(':name')
  findOne(@Param('name') name: string) {
    return this.usecases.get(name);
  }

  @ApiOperation({
    summary: 'Create vector store',
    description:
    'Creates a vector store.',
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
    summary: 'Delete vector store',
    description: 'Delete a vector store.',
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

  @ApiOperation({
    summary: 'Modify vector store',
    description: 'Modifies a vector store',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto,
  })
  @Post()
  async update(@Param('id') id: string, @Body() body: any) {
    return this.usecases.update( 
      id,
      body
    )
  }

}
