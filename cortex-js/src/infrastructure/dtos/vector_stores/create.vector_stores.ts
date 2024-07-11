import { VectorStoreEntity } from '@/infrastructure/entities/vector_store.entity';
import { Optional } from '@nestjs/common';
import { ApiProperty } from '@nestjs/swagger';

export class CreateVectorStoresDto implements Partial<VectorStoreEntity> {
  @ApiProperty({
    example: ['id_1'],
    description:
      'A list of File IDs that the vector store should use. Useful for tools like file_search that can access files.',
  })
  @Optional()
  file_ids: string[];

  @ApiProperty({
    example: 'Vector Store Name',
    description: 'The name of the vector store to be created.',
    type: String,
  })
  @Optional()
  name: string;

  @ApiProperty({
    example: 'Vector Store Name',
    description: 'The expiration policy for a vector store.',
    type: String,
  })
  @Optional()
  expires_after: string;

  @ApiProperty({
    example: {
      max_chunk_size_tokens: 800,
      chunk_overlap_tokens: 400,
    },
    description:
      'The chunking strategy used to chunk the file(s). If not set, will use the auto strategy. Only applicable if file_ids is non-empty.',
    type: Object,
  })
  @Optional()
  chunking_strategy: object;

  @ApiProperty({
    example: '{ "key": "value" }',
    description:
      'Set of 16 key-value pairs that can be attached to an object. This can be useful for storing additional information about the object in a structured format. Keys can be a maximum of 64 characters long and values can be a maxium of 512 characters long.',
    type: Object,
  })
  @Optional()
  metadata: Record<string, string>;
}
