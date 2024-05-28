import { ApiProperty } from '@nestjs/swagger';
import { IsBoolean, IsString } from 'class-validator';
import { AssistantTool } from '@/domain/models/assistant.interface';

export class AssistantToolDto implements AssistantTool {
  @ApiProperty({
    description: "The type of the assistant's tool.",
  })
  @IsString()
  type: string;

  @ApiProperty({
    description: "Enable or disable the assistant's tool.",
  })
  @IsBoolean()
  enabled: boolean;

  // TODO: NamH make a type for this
  @ApiProperty({
    description: "The setting of the assistant's tool.",
  })
  @ApiProperty()
  settings: any;
}
