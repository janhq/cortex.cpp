import { ApiProperty } from '@nestjs/swagger';
import { IsBoolean, IsString } from 'class-validator';
import { AssistantTool } from 'src/domain/models/assistant.interface';

export class AssistantToolDto implements AssistantTool {
  @IsString()
  type: string;

  @IsBoolean()
  enabled: boolean;

  // TODO: NamH make a type for this
  @ApiProperty()
  settings: any;
}
