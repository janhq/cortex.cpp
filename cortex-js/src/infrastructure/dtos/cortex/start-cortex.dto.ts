import { IsIP, IsString } from 'class-validator';

export class StartCortexDto {
  @IsString()
  @IsIP()
  host: string;

  @IsString()
  port: string;
}
