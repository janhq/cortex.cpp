import { Inject, Injectable } from '@nestjs/common';
import { Repository } from 'sequelize-typescript';
import { FileEntity } from '@/infrastructure/entities/file.entity';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { createWriteStream, existsSync, mkdirSync } from 'fs';
import { join } from 'path';
import { ulid } from 'ulid';

@Injectable()
export class FilesUsecases {
  constructor(
    @Inject('FILE_REPOSITORY')
    private readonly filesRepository: Repository<FileEntity>,
    private readonly fileService: FileManagerService,
  ) {}

  /**
   */
  async create(entity: Partial<FileEntity>, file: any) {
    const res = await this.filesRepository.create({
      id: ulid(),
      ...entity,
      object: 'file',
      filename: file.originalname,
    });
    const filesDir = join(await this.fileService.getDataFolderPath(), 'files');
    if (!existsSync(filesDir)) mkdirSync(filesDir);
    const ws = createWriteStream(join(filesDir, res.id));
    ws.write(file.buffer);
    return res;
  }

  /**
   */
  get(name: string) {
    return this.filesRepository.findOne({
      where: {
        name,
      },
    });
  }

  /**
   **/
  remove(name: string) {
    return this.filesRepository.destroy({
      where: {
        name,
      },
    });
  }

  /**
   */
  getAll() {
    return this.filesRepository.findAll();
  }
}
