import {
  Body,
  Controller,
  Post,
  HttpCode,
  Get,
  Param,
  Delete,
  UseInterceptors,
  UploadedFile,
} from '@nestjs/common';
import { FileInterceptor } from '@nestjs/platform-express';
import { ApiOperation, ApiTags, ApiResponse, ApiParam } from '@nestjs/swagger';
import { CommonResponseDto } from '../dtos/common/common-response.dto';
import { FilesUsecases } from '@/usecases/files/files.usecases';
import { FileEntity } from '../entities/file.entity';

@ApiTags('Files')
@Controller('files')
export class FilesController {
  constructor(private readonly usecases: FilesUsecases) {}

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: [FileEntity],
  })
  @ApiOperation({
    summary: 'List of uploaded files',
    description: 'List of uploaded files',
  })
  @Get()
  findAll() {
    return this.usecases.getAll();
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: FileEntity,
  })
  @ApiOperation({
    summary: 'Get an uploaded file',
    description: 'Retrieves an uploaded file',
  })
  @ApiParam({
    name: 'name',
    required: true,
    description: 'The unique identifier of the file.',
  })
  @Get(':name(*)')
  findOne(@Param('name') name: string) {
    return this.usecases.get(name);
  }

  @ApiOperation({
    summary: 'Create a file.',
    description: '',
  })
  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'Ok',
    type: Object,
  })
  @Post()
  @UseInterceptors(FileInterceptor('file'))
  async create(
    @Body() createFile: Partial<FileEntity>,
    @UploadedFile() file: Express.Multer.File,
  ) {
    return this.usecases.create(createFile, file);
  }

  @ApiOperation({
    summary: 'Delete a file',
    description: 'Delete an uploaded file',
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
