import { ApiProperty } from '@nestjs/swagger';

export class DownloadModelResponseDto {
    @ApiProperty({
        example: 'Starting download mistral-ins-7b-q4',
        description: 'Message indicates Jan starting download corresponding model.'
    })
    message: string;
}
