import { ApiProperty } from '@nestjs/swagger';

export class GetThreadResponseDto {
    @ApiProperty({ example: 'thread_abc123', description: 'The identifier of the thread.' })
    id: string;

    @ApiProperty({ example: 'thread', description: 'Type of the object' })
    object: string;

    @ApiProperty({ example: 1699014083, description: 'Unix timestamp representing the creation time of the thread.', type: 'integer' })
    created_at: number;

    @ApiProperty({ example: ['assistant-001'], description: 'List of assistants involved in the thread.', type: [String] })
    assistants: string[];

    @ApiProperty({ example: {}, description: 'Metadata associated with the thread.' })
    metadata: object;

    @ApiProperty({ example: [], description: 'List of messages within the thread.', type: [String] })
    messages: string[];
}
