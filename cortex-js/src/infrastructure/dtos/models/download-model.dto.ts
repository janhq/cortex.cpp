import { IsString } from 'class-validator';

export class DownloadModelDto {
  @IsString()
  modelId: string;
}
