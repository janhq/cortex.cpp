import { Inject, Injectable } from '@nestjs/common';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Repository } from 'sequelize-typescript';
import { VectorStoreEntity } from '@/infrastructure/entities/vector_store.entity';
import { RagExtension } from '@/domain/abstracts/rag.extension.abstract';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { basename, join } from 'path';
import { cpSync, mkdirSync } from 'fs';
import { ulid } from 'ulid';

@Injectable()
export class VectorStoresUsecases {
  constructor(
    private readonly extensionRepository: ExtensionRepository,
    private readonly fileStoreService: FileManagerService,
    @Inject('VECTOR_STORE_REPOSITORY')
    private vectorStoreRepository: Repository<VectorStoreEntity>,
  ) {}

  /**
   * Create a new vector store
   * @param entity Vector store entity
   * @returns
   */
  async create(entity: Partial<VectorStoreEntity>) {
    if (!entity.name) throw new Error('Vector store name is required');
    // Create vector store folder
    mkdirSync(
      join(
        await this.fileStoreService.getVectorStoresFolderPath(),
        entity.name,
      ),
      {
        recursive: true,
      },
    );

    if (
      entity.rag_extension &&
      !(await this.extensionRepository.findOne(entity.rag_extension))
    )
      throw new Error('RAG extension not found');
    // Persist vector store entity
    return this.vectorStoreRepository.create({
      id: ulid(),
      ...entity,
    });
  }

  /**
   * Retrieve a vector store by id
   * @param id Vector store ID
   * @returns
   */
  get(name: string) {
    return this.vectorStoreRepository.findOne({
      where: {
        name,
      },
    });
  }

  /**
   * Delete a vector store by id
   **/
  remove(name: string) {
    return this.vectorStoreRepository.destroy({
      where: {
        name,
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
   * Upload files to a vector store
   * @param files
   * @param vectorStoreId
   */
  async uploadFiles(vectorStoreId: string, files: string[]) {
    const vectorStore = await this.get(vectorStoreId);
    if (!vectorStore) {
      throw new Error('Vector store not found');
    }
    const ragExtension = (await this.extensionRepository.findOne(
      vectorStore.rag_extension,
    )) as unknown as RagExtension | undefined;
    if (!ragExtension) {
      throw new Error('RAG extension not found');
    }

    // Persist files

    // Create cache for additional operations before ingesting
    const cacheDir = join(
      await this.fileStoreService.getVectorStoresFolderPath(),
      vectorStore.name,
      'caches',
      new Date().toISOString(),
    );

    mkdirSync(cacheDir, {
      recursive: true,
    });

    for (const file of files) {
      cpSync(file, join(cacheDir, basename(file)));
    }

    // TODO: Scan for security threats

    // Ingest the folder
    await ragExtension.fromFiles(
      [cacheDir],
      join(
        await this.fileStoreService.getVectorStoresFolderPath(),
        vectorStore.name,
      ),
      vectorStore.vector_database,
    );
  }
}
