import { ApiProperty } from '@nestjs/swagger';

export class DeleteThreadResponseDto {
    @ApiProperty({
        example: 'thread_123',
        description: 'The identifier of the thread that was deleted.'
    })
    id: string;

    @ApiProperty({
        example: 'thread',
        description: 'Type of the object, indicating it\'s a thread.',
        default: 'thread'
    })
    object: string;

    @ApiProperty({
        example: true,
        description: 'Indicates whether the thread was successfully deleted.'
    })
    deleted: boolean;
}
