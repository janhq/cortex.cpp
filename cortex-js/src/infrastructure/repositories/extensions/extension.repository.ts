import { Inject, Injectable } from '@nestjs/common';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Extension } from '@/domain/abstracts/extension.abstract';
import { readdir, lstat } from 'fs/promises';
import { join } from 'path';
import { EngineExtension } from '@/domain/abstracts/engine.abstract';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { existsSync } from 'fs';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';
import { OAIEngineExtension } from '@/domain/abstracts/oai.abstract';

@Injectable()
export class ExtensionRepositoryImpl implements ExtensionRepository {
  // Initialize the Extensions Map with the key-value pairs of the core providers.
  extensions = new Map<string, Extension>();

  constructor(
    @Inject('CORTEX_PROVIDER')
    private readonly cortexProvider: EngineExtension,
    private readonly fileService: FileManagerService,
    @Inject('EXTENSIONS_PROVIDER')
    private readonly coreExtensions: OAIEngineExtension[],
  ) {
    this.loadCoreExtensions();
    this.loadExternalExtensions();
  }
  create(object: Extension): Promise<Extension> {
    this.extensions.set(object.name ?? '', object);
    return Promise.resolve(object);
  }
  findAll(): Promise<Extension[]> {
    return Promise.resolve(
      Array.from(this.extensions.keys()).map(
        (e) =>
          ({
            name: e,
          }) as Extension,
      ),
    );
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

  private async loadCoreExtensions() {
    await this.cortexProvider.onLoad();
    this.extensions.set(Engines.llamaCPP, this.cortexProvider);
    this.extensions.set(Engines.onnx, this.cortexProvider);
    this.extensions.set(Engines.tensorrtLLM, this.cortexProvider);
    for (const extension of this.coreExtensions) {
      await extension.onLoad();
      this.extensions.set(extension.provider, extension);
    }
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
