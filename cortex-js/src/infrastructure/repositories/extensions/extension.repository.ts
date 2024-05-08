import { Injectable } from '@nestjs/common';
import { LazyModuleLoader } from '@nestjs/core';
import { ExtensionRepository } from 'src/domain/repositories/extension.interface';
import { Extension } from '@janhq/core';
import { readdir, lstat } from 'fs/promises';

@Injectable()
export class ExtensionRepositoryImpl implements ExtensionRepository {
  extensions = new Map<string, Extension>();

  constructor(private lazyModuleLoader: LazyModuleLoader) {
    this.loadExtensions();
  }
  create(object: Extension): Promise<Extension> {
    this.extensions.set(object.name ?? '', object);
    return Promise.resolve(object);
  }
  findAll(): Promise<Extension[]> {
    return Promise.resolve(Array.from(this.extensions.values()));
  }
  findOne(id: string): Promise<Extension | null> {
    return Promise.resolve(this.extensions.get(id) ?? null);
  }
  update(): Promise<void> {
    throw new Error('Method not implemented.');
  }
  remove(id: string): Promise<void> {
    this.extensions.delete(id);
    return Promise.resolve();
  }

  loadExtensions(): void {
    const extensionsPath = process.env.EXTENSIONS_PATH ?? 'extensions';
    readdir(extensionsPath).then((files) => {
      files.forEach(async (extension) => {
        if (!(await lstat(`${extensionsPath}/${extension}`)).isDirectory())
          return;

        import(`${extensionsPath}/${extension}`).then((extensionClass) => {
          const newExtension = new extensionClass.default();
          this.extensions.set(extension, newExtension);
        });
      });
    });
  }
}
