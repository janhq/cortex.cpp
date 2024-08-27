import {
  Body,
  Controller,
  DefaultValuePipe,
  Delete,
  Get,
  Param,
  Post,
  Query,
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
  ApiResponse,
  ApiQuery,
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
    description: 'Returns a list of assistants.',
  })
  @ApiOkResponse({
    description: 'Ok',
    type: [AssistantEntity],
  })
  @ApiQuery({
    name: 'limit',
    type: Number,
    required: false,
    description:
      'A limit on the number of objects to be returned. Limit can range between 1 and 100, and the default is 20.',
  })
  @ApiQuery({
    name: 'order',
    type: String,
    required: false,
    description:
      'Sort order by the created_at timestamp of the objects. asc for ascending order and desc for descending order.',
  })
  @ApiQuery({
    name: 'after',
    type: String,
    required: false,
    description:
      'A cursor for use in pagination. after is an object ID that defines your place in the list. For instance, if you make a list request and receive 100 objects, ending with obj_foo, your subsequent call can include after=obj_foo in order to fetch the next page of the list.',
  })
  @ApiQuery({
    name: 'before',
    type: String,
    required: false,
    description:
      'A cursor for use in pagination. before is an object ID that defines your place in the list. For instance, if you make a list request and receive 100 objects, ending with obj_foo, your subsequent call can include before=obj_foo in order to fetch the previous page of the list.',
  })
  @Get()
  findAll(
    @Query('limit', new DefaultValuePipe(20)) limit: number,
    @Query('order', new DefaultValuePipe('desc')) order: 'asc' | 'desc',
    @Query('after') after?: string,
    @Query('before') before?: string,
  ) {
    return this.assistantsService.listAssistants(limit, order, after, before);
  }

  @ApiOperation({
    summary: 'Get assistant',
    description:
      "Retrieves a specific assistant defined by an assistant's `id`.",
  })
  @ApiOkResponse({
    description: 'Ok',
    type: AssistantEntity,
  })
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The unique identifier of the assistant.',
  })
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
  @ApiParam({
    name: 'id',
    required: true,
    description: 'The unique identifier of the assistant.',
  })
  @Delete(':id')
  remove(@Param('id') id: string) {
    return this.assistantsService.remove(id);
  }
}
