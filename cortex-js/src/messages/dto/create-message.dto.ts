import { IsEmail } from 'class-validator';

export class CreateMessageDto {
  @IsEmail()
  email: string;
}
