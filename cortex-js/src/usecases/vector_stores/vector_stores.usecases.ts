import { Inject, Injectable } from '@nestjs/common';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Repository } from 'sequelize-typescript';
import { VectorStoreEntity } from '@/infrastructure/entities/vector_store.entity';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { join } from 'path';
import { mkdirSync } from 'fs';
import { ulid } from 'ulid';

@Injectable()
export class VectorStoresUsecases {
  constructor(
    private readonly extensionRepository: ExtensionRepository,
    private readonly fileStoreService: FileManagerService,
    @Inject('VECTOR_STORE_REPOSITORY')
    private readonly vectorStoreRepository: Repository<VectorStoreEntity>,
  ) {}

  /**
   * Create a new vector store
   * @param entity Vector store entity
   * @returns
   */
  async create(entity: Partial<VectorStoreEntity>) {
    const vs = await this.vectorStoreRepository.create({
      id: ulid(),
      ...entity,
      rag_extension: entity?.metadata?.rag_extension ?? 'llamaindex',
      vector_database: entity?.metadata?.vector_database ?? 'default',
    });

    // Create vector store folder
    mkdirSync(
      join(
        await this.fileStoreService.getVectorStoresFolderPath(),
        vs.id,
      ),
      {
        recursive: true,
      },
    );

    if (
      entity?.metadata?.rag_extension &&
        !(await this.extensionRepository.findOne(entity.metadata.rag_extension)
        ))
    throw new Error('RAG extension not found');
    
    return vs;
  }

  /**
   * Retrieve a vector store by id
   * @param id Vector store ID
   * @returns
   */
  get(id: string) {
    return this.vectorStoreRepository.findByPk(id);
  }

  /**
   * Delete a vector store by id
   **/
  remove(id: string) {
    return this.vectorStoreRepository.destroy({
      where: {
        id,
      },
    });
  }

  /**
   * Retrieve all vector stores
   * @returns
   */
  getAll() {
    return this.vectorStoreRepository.findAll();
  }

  /**
   * Modifies a vector store
   */
  update(id: string, entity: Partial<VectorStoreEntity>) {
    return this.vectorStoreRepository.update(entity, {
      where: { id }
    });
  }
}
