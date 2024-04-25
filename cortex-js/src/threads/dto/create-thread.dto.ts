import { IsString } from 'class-validator';

export class CreateThreadDto {
  @IsString()
  title: string;
}
