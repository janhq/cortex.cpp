import { Body, Controller, Delete, Get, Param, Post } from '@nestjs/common';
import { AssistantsService } from './assistants.service';
import { CreateAssistantDto } from './dto/create-assistant.dto';

@Controller('assistants')
export class AssistantsController {
  constructor(private readonly assistantsService: AssistantsService) {}

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
