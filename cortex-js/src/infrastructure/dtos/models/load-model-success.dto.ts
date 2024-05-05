import { IsString } from 'class-validator';

export class LoadModelSuccessDto {
  @IsString()
  message: string;

  @IsString()
  modelId: string;
}
