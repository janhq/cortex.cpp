import { ApiProperty } from '@nestjs/swagger';

export class ContentDto {
    @ApiProperty({ example: 'text', description: 'Type of content, e.g., "text".' })
    type: string;

    @ApiProperty({
        type: 'object',
        example: { value: "How does AI work? Explain it in simple terms.", annotations: [] },
        description: 'Text content of the message along with any annotations.'
    })
    text: {
        value: string;
        annotations: string[];
    };
}

export class GetMessageResponseDto {
    @ApiProperty({ example: 'msg_abc123', description: 'The identifier of the message.' })
    id: string;

    @ApiProperty({ example: 'thread.message', description: "Type of the object, indicating it's a thread message.", default: 'thread.message' })
    object: string;

    @ApiProperty({ example: 1699017614, description: 'Unix timestamp representing the creation time of the message.', type: 'integer' })
    created_at: number;

    @ApiProperty({ example: 'thread_abc123', description: 'Identifier of the thread to which this message belongs.' })
    thread_id: string;

    @ApiProperty({ example: 'user', description: "Role of the sender, either 'user' or 'assistant'." })
    role: string;

    @ApiProperty({ type: [ContentDto], description: 'Array of content objects detailing the message content.' })
    content: ContentDto[];

    @ApiProperty({ example: [], description: 'Array of file IDs associated with the message, if any.', type: [String] })
    file_ids: string[];

    @ApiProperty({ nullable: true, example: null, description: 'Identifier of the assistant involved in the message, if applicable.' })
    assistant_id: string | null;

    @ApiProperty({ nullable: true, example: null, description: 'Run ID associated with the message, if applicable.' })
    run_id: string | null;

    @ApiProperty({ example: {}, description: 'Metadata associated with the message.' })
    metadata: object;
}
