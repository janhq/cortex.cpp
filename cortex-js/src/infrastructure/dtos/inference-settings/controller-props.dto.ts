import { IsOptional, IsString } from 'class-validator';
import { ControllerProps } from '@/domain/models/inference-setting.interface';

export class ControllerPropsDto implements ControllerProps {
  @IsString()
  placeholder: string;

  @IsString()
  value: string;

  @IsOptional()
  @IsString()
  type?: string;
}
