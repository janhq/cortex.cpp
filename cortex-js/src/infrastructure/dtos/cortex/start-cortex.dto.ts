import { IsIP, IsNumber, IsString, Max, Min } from 'class-validator';

export class StartCortexDto {
  @IsString()
  @IsIP()
  host: string;

  @IsNumber()
  @Min(0)
  @Max(65535)
  port: number;
}
