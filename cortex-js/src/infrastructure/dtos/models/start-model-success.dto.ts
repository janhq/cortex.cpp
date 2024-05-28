import { IsString } from 'class-validator';
import { ApiProperty } from '@nestjs/swagger';

export class StartModelSuccessDto {
  @ApiProperty({description: "The success or error message displayed when a model is successfully loaded or fails to load."})
  @IsString()
  message: string;

  @ApiProperty({description: "The unique identifier of the model."})
  @IsString()
  modelId: string;
}
