import { Inject, Injectable } from '@nestjs/common';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Extension } from '@/domain/abstracts/extension.abstract';
import { readdir, lstat } from 'fs/promises';
import { join } from 'path';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { appPath } from '@/utils/app-path';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { existsSync } from 'fs';

@Injectable()
export class ExtensionRepositoryImpl implements ExtensionRepository {
  // Initialize the Extensions Map with the key-value pairs of the core providers.
  extensions = new Map<string, Extension>([
    ['cortex.llamacpp', this.cortexProvider],
    ['cortex.onnx', this.cortexProvider],
  ]);

  constructor(
    @Inject('CORTEX_PROVIDER')
    private readonly cortexProvider: EngineExtension,
    private readonly fileService: FileManagerService,
  ) {
    this.loadCoreExtensions();
    this.loadExternalExtensions();
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

  private loadCoreExtensions(): void {
    const extensionsPath = join(appPath, 'src', 'extensions');
    this.loadExtensions(extensionsPath);
  }

  private async loadExternalExtensions() {
    const extensionsPath =
      process.env.EXTENSIONS_PATH ??
      (await this.fileService.getExtensionsPath());
    this.loadExtensions(extensionsPath);
  }

  private async loadExtensions(extensionsPath: string) {
    if (!existsSync(extensionsPath)) return;

    readdir(extensionsPath).then((files) => {
      files.forEach(async (extension) => {
        const extensionFullPath = join(extensionsPath, extension);
        if (!(await lstat(extensionFullPath)).isDirectory()) return;

        import(extensionFullPath).then((extensionClass) => {
          const newExtension = new extensionClass.default();
          this.extensions.set(extension, newExtension);
        });
      });
    });
  }
}
