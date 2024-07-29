import { Inject, Injectable } from '@nestjs/common';
import { ExtensionRepository } from '@/domain/repositories/extension.interface';
import { Extension } from '@/domain/abstracts/extension.abstract';
import { readdir, lstat } from 'fs/promises';
import { join } from 'path';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { existsSync } from 'fs';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';
import { OAIEngineExtension } from '@/domain/abstracts/oai.abstract';
import { HttpService } from '@nestjs/axios';
import LlamaCPPProvider from '@/infrastructure/providers/cortex/llamacpp.provider';
import Onnxprovider from '@/infrastructure/providers/cortex/onnx.provider';
import TensorrtLLMProvider from '@/infrastructure/providers/cortex/tensorrtllm.provider';
import { EngineStatus } from '@/domain/abstracts/engine.abstract';

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
  /**
   * Persist extension to the extensions map
   * @param object 
   * @returns 
   */
  create(object: Extension): Promise<Extension> {
    this.extensions.set(object.name ?? '', object);
    return Promise.resolve(object);
  }

  /**
   * Find all extensions
   * @returns 
   */
  findAll(): Promise<Extension[]> {
    return Promise.resolve(Array.from(this.extensions.values()));
  }

  /**
   * Find one extension by id
   * @param id 
   * @returns 
   */
  findOne(id: string): Promise<Extension | null> {
    return Promise.resolve(this.extensions.get(id) ?? null);
  }

  /**
   * Update extension
   * It is not applicable
   */
  update(): Promise<void> {
    throw new Error('Method not implemented.');
  }

  /**
   * Remove extension from the extensions map
   * @param id 
   * @returns 
   */
  remove(id: string): Promise<void> {
    this.extensions.delete(id);
    return Promise.resolve();
  }

  private async loadCoreExtensions() {
    const llamaCPPEngine = new LlamaCPPProvider(
      this.httpService,
      this.fileManagerService,
    );
    llamaCPPEngine.status = existsSync(
      join(
        await this.fileManagerService.getCortexCppEnginePath(),
        Engines.llamaCPP,
      ),
    )
      ? EngineStatus.READY
      : EngineStatus.NOT_INITIALIZED;

    const onnxEngine = new Onnxprovider(
      this.httpService,
      this.fileManagerService,
    );
    onnxEngine.status =
      existsSync(
        join(
          await this.fileManagerService.getCortexCppEnginePath(),
          Engines.onnx,
        ),
      ) && process.platform === 'win32'
        ? EngineStatus.READY
        : process.platform !== 'win32'
          ? EngineStatus.NOT_SUPPORTED
          : EngineStatus.NOT_INITIALIZED;

    const tensorrtLLMEngine = new TensorrtLLMProvider(
      this.httpService,
      this.fileManagerService,
    );
    tensorrtLLMEngine.status =
      existsSync(
        join(
          await this.fileManagerService.getCortexCppEnginePath(),
          Engines.tensorrtLLM,
        ),
      ) && process.platform !== 'darwin'
        ? EngineStatus.READY
        : process.platform === 'darwin'
          ? EngineStatus.NOT_SUPPORTED
          : EngineStatus.NOT_INITIALIZED;

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
