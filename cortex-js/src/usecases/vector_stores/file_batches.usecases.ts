import { RagExtension } from "@/domain/abstracts/rag.extension.abstract";
import { ExtensionRepository } from "@/domain/repositories/extension.interface";
import { FileEntity } from "@/infrastructure/entities/file.entity";
import { FileBatchEntity } from "@/infrastructure/entities/file_batch.entity";
import { VectorStoreEntity } from "@/infrastructure/entities/vector_store.entity";
import { FileManagerService } from "@/infrastructure/services/file-manager/file-manager.service";
import { Inject, Injectable } from "@nestjs/common";
import { existsSync, mkdirSync, symlinkSync } from "fs";
import { extname, join } from "path";
import { Repository } from "sequelize-typescript";
import { ulid } from "ulid";

@Injectable()
export class FileBatchesUsecases {
  constructor(
    @Inject('FILE_BATCH_REPOSITORY')
    private readonly repository: Repository<FileBatchEntity>,
    private readonly fileStoreService: FileManagerService,
    private readonly extensionRepository: ExtensionRepository,
    @Inject('FILE_REPOSITORY')
    private readonly fileRepository: Repository<FileEntity>,
    @Inject('VECTOR_STORE_REPOSITORY')
    private readonly vectorStoreRepository: Repository<VectorStoreEntity>,
  ) {}

  async create(vectorStoreId: string, batch: { file_ids: string[], chunking_strategy: string }) {
    try {
      const fileBatchRecord = await this.repository.create({
        id: ulid(),
        vector_store_id: vectorStoreId,
        object: 'vector_store.files_batch',
        status: 'in_progress',
        file_counts: {
          in_progress: batch.file_ids.length,
          completed: 0,
          failed: 0,
          cancelled: 0,
          total: batch.file_ids.length
        }
      })
      // Put into a queue - non blocking API
      const vectorStore = await this.vectorStoreRepository.findByPk(vectorStoreId);
      if (!vectorStore) {
        throw new Error('Vector store not found');
      }
      const ragExtension = (await this.extensionRepository.findOne(
        vectorStore.rag_extension,
      )) as unknown as RagExtension | undefined;

      if (!ragExtension) {
        throw new Error('RAG extension not found');
      }

      const vectorStoreDir = join(
        await this.fileStoreService.getVectorStoresFolderPath(),
        vectorStoreId,
        'files',
      );

      if (!existsSync(vectorStoreDir)) mkdirSync(vectorStoreDir);

      for (const file of batch.file_ids) {
        const fileEntity = await this.fileRepository.findOne({
          where: {
            id: file,
          },
        });

        if (!fileEntity) continue;

        if(!existsSync(join(vectorStoreDir, `${file}${extname(fileEntity.filename)}`)))
        // create symlink to original file with file extension under the same folder
        symlinkSync(
          join(await this.fileStoreService.getFilesFolderPath(), file),
          join(vectorStoreDir, `${file}${extname(fileEntity.filename)}`),
        );
      }

      // Ingest the folder
      await ragExtension.fromFiles(
        [vectorStoreDir],
        join(
          await this.fileStoreService.getVectorStoresFolderPath(),
          vectorStore.id,
        ),
        vectorStore.vector_database,
      );
      await this.update(fileBatchRecord.id, {
        file_counts: {
          completed: batch.file_ids.length,
          failed: 0,
          cancelled: 0,
          total: batch.file_ids.length
        },
        status: 'completed',
      })

      return this.get(fileBatchRecord.id)
    } catch(ex) {console.log(ex); throw ex}
  }

  get(id: string) {
    return this.repository.findByPk(id)
  }

  remove(id: string) {
    return this.repository.destroy({
      where: { id }
    })
  }

  getAll(vectorStoreId: string) {
    return this.repository.findAll({
      where: {  vector_store_id: vectorStoreId }
    })
  }

  update(id: string, entity: Partial<FileBatchEntity>) {
    this.repository.update(entity, {
      where: { id }
    })
  }
}
