import { IsString } from 'class-validator';

export class StartModelSuccessDto {
  @IsString()
  message: string;

  @IsString()
  modelId: string;
}
