import { ApiProperty } from '@nestjs/swagger';

export class DeleteModelResponseDto {
    @ApiProperty({
        example: 'mistral-ins-7b-q4',
        description: 'The identifier of the model that was deleted.'
    })
    id: string;

    @ApiProperty({
        example: 'model',
        description: 'Type of the object, indicating it\'s a model.',
        default: 'model'
    })
    object: string;

    @ApiProperty({
        example: true,
        description: 'Indicates whether the model was successfully deleted.'
    })
    deleted: boolean;
}
