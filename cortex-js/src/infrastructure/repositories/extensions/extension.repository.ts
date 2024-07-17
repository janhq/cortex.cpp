import { Inject, Injectable } from '@nestjs/common';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Extension } from '@/domain/abstracts/extension.abstract';
import { readdir, lstat } from 'fs/promises';
import { join } from 'path';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { existsSync } from 'fs';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';
import { OAIEngineExtension } from '@/domain/abstracts/oai.abstract';
import CortexProvider from '@/infrastructure/providers/cortex/cortex.provider';
import { HttpService } from '@nestjs/axios';

@Injectable()
export class ExtensionRepositoryImpl implements ExtensionRepository {
  // Initialize the Extensions Map with the key-value pairs of the core providers.
  extensions = new Map<string, Extension>();

  constructor(
    private readonly fileService: FileManagerService,
    @Inject('EXTENSIONS_PROVIDER')
    private readonly coreExtensions: OAIEngineExtension[],
    private readonly httpService: HttpService,
    private readonly fileManagerService: FileManagerService,
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

  private async loadCoreExtensions() {
    const llamaCPPEngine = new CortexProvider(
      this.httpService,
      this.fileManagerService,
    );
    llamaCPPEngine.name = Engines.llamaCPP;
    llamaCPPEngine.initalized = existsSync(
      join(
        await this.fileManagerService.getCortexCppEnginePath(),
        Engines.llamaCPP,
      ),
    );

    const onnxEngine = new CortexProvider(
      this.httpService,
      this.fileManagerService,
    );
    onnxEngine.name = Engines.onnx;
    onnxEngine.initalized = existsSync(
      join(
        await this.fileManagerService.getCortexCppEnginePath(),
        Engines.onnx,
      ),
    );

    const tensorrtLLMEngine = new CortexProvider(
      this.httpService,
      this.fileManagerService,
    );
    tensorrtLLMEngine.name = Engines.tensorrtLLM;
    tensorrtLLMEngine.initalized = existsSync(
      join(
        await this.fileManagerService.getCortexCppEnginePath(),
        Engines.tensorrtLLM,
      ),
    );

    await llamaCPPEngine.onLoad();
    await onnxEngine.onLoad();
    await tensorrtLLMEngine.onLoad();

    this.extensions.set(Engines.llamaCPP, llamaCPPEngine);
    this.extensions.set(Engines.onnx, onnxEngine);
    this.extensions.set(Engines.tensorrtLLM, tensorrtLLMEngine);

    for (const extension of this.coreExtensions) {
      await extension.onLoad();
      this.extensions.set(extension.name, extension);
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
