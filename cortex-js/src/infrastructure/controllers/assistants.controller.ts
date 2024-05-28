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
import { DeleteAssistantResponseDto } from '@/infrastructure/dtos/assistants/delete-assistant.dto';
import {
  ApiCreatedResponse,
  ApiOkResponse,
  ApiOperation,
  ApiParam,
  ApiTags,
  ApiResponse
} from '@nestjs/swagger';
import { AssistantEntity } from '../entities/assistant.entity';
import { TransformInterceptor } from '../interceptors/transform.interceptor';

@ApiTags('Assistants')
@Controller('assistants')
@UseInterceptors(TransformInterceptor)
export class AssistantsController {
  constructor(private readonly assistantsService: AssistantsUsecases) {}

  @ApiOperation({
    summary: 'Create assistant',
    description: 'Creates a new assistant.',
  })
  @ApiCreatedResponse({
    description: 'The assistant has been successfully created.',
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
    description: 'Ok',
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
    description: 'Ok',
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
  @ApiResponse({
    status: 200,
    description: 'The assistant has been successfully deleted.',
    type: DeleteAssistantResponseDto,
  })
  @ApiParam({ name: 'id', required: true, description: "The unique identifier of the assistant." })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.assistantsService.remove(id);
  }
}
