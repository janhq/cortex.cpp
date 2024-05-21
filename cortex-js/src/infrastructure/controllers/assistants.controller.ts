import { Body, Controller, Delete, Get, Param, Post } from '@nestjs/common';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { CreateAssistantDto } from '@/infrastructure/dtos/assistants/create-assistant.dto';
import { ApiTags, ApiOperation } from '@nestjs/swagger';

@ApiTags('Assistants')
@Controller('assistants')
export class AssistantsController {
  constructor(private readonly assistantsService: AssistantsUsecases) {}

  @ApiOperation({
    summary: 'Create Assistant',
    description: 'Creates a new assistant.',
  })
  @Post()
  create(@Body() createAssistantDto: CreateAssistantDto) {
    return this.assistantsService.create(createAssistantDto);
  }

  @ApiOperation({
    summary: 'List Assistants',
    description: 'Retrieves all the available assistants along with their settings.',
  })
  @Get()
  findAll() {
    return this.assistantsService.findAll();
  }

  @ApiOperation({
    summary: 'Get Assistant',
    description: "Retrieves a specific assistant defined by an assistant's `id`.",
  })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.assistantsService.findOne(id);
  }

  @ApiOperation({
    summary: 'Delete Assistant',
    description: "Deletes a specific assistant defined by an assistant's `id`.",
  })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.assistantsService.remove(id);
  }
}
