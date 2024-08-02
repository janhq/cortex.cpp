import { Body, Controller, Get, HttpCode, Param, Post } from "@nestjs/common";
import { ApiOperation, ApiResponse, ApiTags } from "@nestjs/swagger";
import { CommonResponseDto } from "../dtos/common/common-response.dto";
import { FileBatchesUsecases } from "@/usecases/vector_stores/file_batches.usecases";

@ApiTags('Vector Store File Batches')
@Controller('vector_stores/:vector_store_id/file_batches')
export class FileBatchesController {

  constructor(private readonly usecases: FileBatchesUsecases) {}
  
  @ApiOperation({
    summary: '',
    description: '',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto,
  })
  @Get()
  async getBatches(@Param('vector_store_id') vectorStoreId: string) {
    return this.usecases.getAll(vectorStoreId)
  }

  @ApiOperation({
    summary: '',
    description: '',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto,
  })
  @Get(':batch_id')
  async getBatch(@Param('batch_id') batchId: string) {
    return this.usecases.get(batchId)
  }

  @ApiOperation({
    summary: '',
    description: '',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: CommonResponseDto
  })
  @Post()
  create(@Param('vector_store_id') vectorStoreId: string, @Body() body: any) {
    return this.usecases.create(vectorStoreId, body)
  } 
}
