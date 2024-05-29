import { ApiProperty } from '@nestjs/swagger';

export class ModelDto {
    @ApiProperty({ example: 'https://huggingface.co/janhq/trinity-v1.2-GGUF/resolve/main/trinity-v1.2.Q4_K_M.gguf', description: 'URL to the source of the model.' })
    source_url: string;

    @ApiProperty({ example: 'trinity-v1.2-7b', description: 'Unique identifier used in chat-completions model_name, matches folder name.' })
    id: string;

    @ApiProperty({ example: 'model' })
    object: string;

    @ApiProperty({ example: 'Trinity-v1.2 7B Q4', description: 'Name of the model.' })
    name: string;

    @ApiProperty({ default: '1.0', description: 'The version number of the model.' })
    version: string;

    @ApiProperty({ example: 'Trinity is an experimental model merge using the Slerp method. Recommended for daily assistance purposes.', description: 'Description of the model.' })
    description: string;

    @ApiProperty({ example: 'gguf', description: 'State format of the model, distinct from the engine.' })
    format: string;

    @ApiProperty({ description: 'Context length.', example: 4096 })
    ctx_len: number;

    @ApiProperty({ example: 'system\n{system_message}\nuser\n{prompt}\nassistant' })
    prompt_template: string;

    @ApiProperty({ example: 0.7 })
    temperature: number;

    @ApiProperty({ example: 0.95 })
    top_p: number;

    @ApiProperty({ example: true })
    stream: boolean;

    @ApiProperty({ example: 4096 })
    max_tokens: number;

    @ApiProperty({ type: [String], example: [] })
    stop: string[];

    @ApiProperty({ example: 0 })
    frequency_penalty: number;

    @ApiProperty({ example: 0 })
    presence_penalty: number;

    @ApiProperty({ example: 'Jan' })
    author: string;

    @ApiProperty({ type: [String], example: ['7B', 'Merged', 'Featured'] })
    tags: string[];

    @ApiProperty({ example: 4370000000 })
    size: number;

    @ApiProperty({ example: 'https://raw.githubusercontent.com/janhq/jan/main/models/trinity-v1.2-7b/cover.png' })
    cover: string;

    @ApiProperty({ example: 'cortex' })
    engine: string;
}
