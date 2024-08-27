import { ApiProperty } from '@nestjs/swagger';
import { ModelDto } from './model.dto'; // Import the ModelDto class

export class ListModelsResponseDto {
  @ApiProperty({ example: 'list', enum: ['list'] })
  object: string;

  @ApiProperty({ type: [ModelDto], description: 'List of models' })
  data: ModelDto[];
}
