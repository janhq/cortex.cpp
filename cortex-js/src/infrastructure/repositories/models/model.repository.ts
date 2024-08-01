import { Injectable } from '@nestjs/common';
import { join, extname, basename } from 'path';
import { ModelRepository } from '@/domain/repositories/model.interface';
import { Model } from '@/domain/models/model.interface';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import {
  existsSync,
  mkdirSync,
  readFileSync,
  readdirSync,
  rmSync,
  writeFileSync,
  watch,
} from 'fs';
import { load, dump } from 'js-yaml';
import { isLocalModel, normalizeModelId } from '@/utils/normalize-model-id';

@Injectable()
export class ModelRepositoryImpl implements ModelRepository {
  // Initialize the Extensions Map with the key-value pairs of the core providers.
  models = new Map<string, Model>([]);
  // Map between files and models. E.g. llama3:7b -> llama3-7b.yaml
  fileModel = new Map<string, string>([]);
  // Check whether the models have been loaded or not.
  loaded = false;

  constructor(private readonly fileService: FileManagerService) {
    this.loadModels();
    fileService.getModelsPath().then((path) => {
      if (!existsSync(path)) mkdirSync(path);
      watch(path, (eventType, filename) => {
        this.loadModels(true);
      });
    });
  }

  /**
   * Create a new model
   * This would persist the model yaml file to the models folder
   * @param object
   * @returns the created model
   */
  async create(object: Model): Promise<Model> {
    const modelsFolderPath = join(
      await this.fileService.getDataFolderPath(),
      'models',
    );
    const modelYaml = dump(object);
    if (!existsSync(modelsFolderPath)) mkdirSync(modelsFolderPath);
    const modelsPath =
      process.env.EXTENSIONS_PATH ?? (await this.fileService.getModelsPath());
    writeFileSync(
      join(modelsPath, `${normalizeModelId(object.model)}.yaml`),
      modelYaml,
    );

    this.models.set(object.model ?? '', object);
    return Promise.resolve(object);
  }

  /**
   * Find all models
   * This would load all the models from the models folder
   * @param object
   * @returns the created model
   */
  findAll(): Promise<Model[]> {
    return this.loadModels().then((res) =>
      res.filter((model) => isLocalModel(model.files)),
    );
  }
  /**
   * Find one model by id
   * @param id model id
   * @returns the model
   */
  findOne(id: string): Promise<Model | null> {
    return this.loadModels().then(() => this.models.get(id) ?? null);
  }

  /**
   * Update a model
   * This would update the model yaml file in the models folder
   * @param id model id
   * @param object model object
   */
  async update(id: string, object: Partial<Model>): Promise<void> {
    const originalModel = await this.findOne(id);
    if (!originalModel) throw new Error('Model not found');

    const updatedModel = {
      ...originalModel,
      ...object,
    } satisfies Model;

    const modelYaml = dump(updatedModel);
    const modelsPath =
      process.env.EXTENSIONS_PATH ?? (await this.fileService.getModelsPath());

    writeFileSync(
      join(
        modelsPath,
        this.fileModel.get(id) ?? `${normalizeModelId(id)}.yaml`,
      ),
      modelYaml,
    );

    this.models.set(id ?? '', updatedModel);
  }

  /**
   * Remove a model
   * This would remove the model yaml file from the models folder
   * @param id model id
   */
  async remove(id: string): Promise<void> {
    this.models.delete(id);
    const yamlFilePath = join(
      await this.fileService.getModelsPath(),
      this.fileModel.get(id) ?? id,
    );
    if (existsSync(yamlFilePath)) rmSync(yamlFilePath);
    return Promise.resolve();
  }

  /**
   * Load all models
   * This would load all the models from the models folder
   * @returns the list of models
   */
  private async loadModels(forceReload: boolean = false): Promise<Model[]> {
    if (this.loaded && !forceReload) return Array.from(this.models.values());
    const modelsPath =
      process.env.EXTENSIONS_PATH ?? (await this.fileService.getModelsPath());

    this.models.clear();
    this.fileModel.clear();

    if (!existsSync(modelsPath)) return [];

    const modelFiles = readdirSync(modelsPath)
      .filter(
        (file) =>
          extname(file).toLowerCase() === '.yaml' ||
          extname(file).toLowerCase() === '.yml',
      )
      .map((file) => join(modelsPath, file));

    modelFiles.forEach(async (modelFile) => {
      const model = readFileSync(modelFile, 'utf8');
      const yamlObject = load(model) as Model;
      const fileName = basename(modelFile);

      if (yamlObject) {
        this.fileModel.set(yamlObject.model, fileName);
        this.models.set(yamlObject.model, yamlObject);
      }
    });
    this.loaded = true;
    return Array.from(this.models.values());
  }
}
