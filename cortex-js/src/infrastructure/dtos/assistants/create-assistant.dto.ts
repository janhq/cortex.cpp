import { IsArray, IsOptional, IsString } from 'class-validator';
import { Assistant } from '@/domain/models/assistant.interface';
import { ApiProperty } from '@nestjs/swagger';

export class CreateAssistantDto implements Partial<Assistant> {
  @ApiProperty({
    description: 'The unique identifier of the assistant.',
    example: 'jan',
    default: 'jan',
  })
  @IsString()
  id: string;

  @ApiProperty({
    description: 'The avatar of the assistant.',
    example: '',
    default: '',
  })
  @IsOptional()
  @IsString()
  avatar?: string;

  @ApiProperty({
    description: 'The name of the assistant.',
    example: 'Jan',
    default: 'Jan',
  })
  @IsString()
  name: string;

  @ApiProperty({
    description: 'The description of the assistant.',
    example: 'A default assistant that can use all downloaded models',
    default: 'A default assistant that can use all downloaded models',
  })
  @IsString()
  description: string;

  @ApiProperty({
    description: 'The model of the assistant.',
  })
  @IsString()
  model: string;

  @ApiProperty({
    description: 'The instructions for the assistant.',
    example: '',
    default: '',
  })
  @IsString()
  instructions: string;

  @ApiProperty({
    description: 'The tools associated with the assistant.',
    example: [],
    default: [],
  })
  @IsArray()
  tools: any[];

  @ApiProperty({
    description: 'The metadata of the assistant.',
  })
  @IsOptional()
  metadata: unknown | null;
}
