import { VectorStoreEntity } from '@/infrastructure/entities/vector_store.entity';
import { ApiProperty } from '@nestjs/swagger';

export class VectorStoreDto implements Partial<VectorStoreEntity> {}
