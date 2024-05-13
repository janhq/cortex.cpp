import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  HttpCode,
} from '@nestjs/common';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { CreateModelDto } from '@/infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { ApiResponse, ApiTags } from '@nestjs/swagger';
import { LoadModelSuccessDto } from '@/infrastructure/dtos/models/load-model-success.dto';
import { LoadModelDto } from '@/infrastructure/dtos/models/load-model.dto';
import { DownloadModelDto } from '@/infrastructure/dtos/models/download-model.dto';

@ApiTags('Models')
@Controller('models')
export class ModelsController {
  constructor(private readonly modelsService: ModelsUsecases) {}

  @Post()
  create(@Body() createModelDto: CreateModelDto) {
    return this.modelsService.create(createModelDto);
  }

  @HttpCode(200)
  @ApiResponse({
    status: 200,
    description: 'The model has been loaded successfully.',
    type: LoadModelSuccessDto,
  })
  @Post('load')
  load(@Body() loadModelDto: LoadModelDto) {
    return this.modelsService.loadModel(loadModelDto);
  }

  @Post('download')
  downloadModel(@Body() downloadModelDto: DownloadModelDto) {
    return this.modelsService.downloadModel(downloadModelDto);
  }

  @Get()
  findAll() {
    return this.modelsService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.modelsService.findOne(id);
  }

  @Patch(':id')
  update(@Param('id') id: string, @Body() updateModelDto: UpdateModelDto) {
    return this.modelsService.update(id, updateModelDto);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.modelsService.remove(id);
  }
}
