import { PartialType } from '@nestjs/mapped-types';
import { CreateModelDto } from './create-model.dto';

export class UpdateModelDto extends PartialType(CreateModelDto) {}
