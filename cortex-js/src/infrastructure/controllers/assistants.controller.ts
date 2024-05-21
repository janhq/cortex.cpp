import { Body, Controller, Delete, Get, Param, Post } from '@nestjs/common';
import { AssistantsUsecases } from '@/usecases/assistants/assistants.usecases';
import { CreateAssistantDto } from '@/infrastructure/dtos/assistants/create-assistant.dto';
import {
  ApiCreatedResponse,
  ApiOkResponse,
  ApiOperation,
  ApiParam,
  ApiTags,
} from '@nestjs/swagger';
import { AssistantEntity } from '../entities/assistant.entity';

@ApiTags('Assistants')
@Controller('assistants')
export class AssistantsController {
  constructor(private readonly assistantsService: AssistantsUsecases) {}

  @ApiOperation({
    summary: 'Create assistant',
    description: 'Creates a new assistant.',
  })
  @ApiCreatedResponse({
    description: 'Assistant created successfully.',
  })
  @Post()
  create(@Body() createAssistantDto: CreateAssistantDto) {
    return this.assistantsService.create(createAssistantDto);
  }

  @ApiOperation({
    summary: 'List assistants',
    description: 'Retrieves all the available assistants along with their settings.',
  })
  @ApiOkResponse({
    description: 'Return an array of assistants',
    type: [AssistantEntity],
  })
  @Get()
  findAll() {
    return this.assistantsService.findAll();
  }

  @ApiOperation({
    summary: 'Get assistant',
    description: "Retrieves a specific assistant defined by an assistant's `id`.",
  })
  @ApiOkResponse({
    description: 'Return an assistant object',
    type: AssistantEntity,
  })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the assistant." })
  @Get(':id')
  findOne(@Param('id') id: string) {
    return this.assistantsService.findOne(id);
  }

  @ApiOperation({
    summary: 'Delete assistant',
    description: "Deletes a specific assistant defined by an assistant's `id`.",
  })
  @ApiOkResponse({
    description: 'Successfully deleted the assistant.',
  })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the assistant." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.assistantsService.remove(id);
  }
}
