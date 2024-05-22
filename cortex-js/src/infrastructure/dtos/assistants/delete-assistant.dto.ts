import { ApiProperty } from '@nestjs/swagger';

export class DeleteAssistantResponseDto {
    @ApiProperty({
        example: 'assistant_123',
        description: 'The identifier of the assistant that was deleted.'
    })
    id: string;

    @ApiProperty({
        example: 'assistant',
        description: 'Type of the object, indicating it\'s a assistant.',
        default: 'assistant'
    })
    object: string;

    @ApiProperty({
        example: true,
        description: 'Indicates whether the assistant was successfully deleted.'
    })
    deleted: boolean;
}
