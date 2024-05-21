import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  Post,
  UseInterceptors,
} from '@nestjs/common';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { CreateAssistantDto } from '@/infrastructure/dtos/assistants/create-assistant.dto';
import { ApiTags } from '@nestjs/swagger';
import { TransformInterceptor } from '../interceptors/transform.interceptor';

@ApiTags('Assistants')
@Controller('assistants')
@UseInterceptors(TransformInterceptor)
export class AssistantsController {
  constructor(private readonly assistantsService: AssistantsUsecases) {}

  @Post()
  create(@Body() createAssistantDto: CreateAssistantDto) {
    return this.assistantsService.create(createAssistantDto);
  }

  @Get()
  findAll() {
    return this.assistantsService.findAll();
  }

  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.assistantsService.findOne(id);
  }

  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.assistantsService.remove(id);
  }
}
