import { exit } from 'node:process';
import { ModelsUsecases } from '@/usecases/models/models.usecases';
import { Model } from '@/domain/models/model.interface';
import { InquirerService } from 'nest-commander';
import { Inject, Injectable } from '@nestjs/common';

import { StartModelSuccessDto } from '@/infrastructure/dtos/models/start-model-success.dto';
import { UpdateModelDto } from '@/infrastructure/dtos/models/update-model.dto';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';
import { join } from 'path';
import { load } from 'js-yaml';
import { existsSync, readdirSync, readFileSync } from 'fs';
import { isLocalModel } from '@/utils/normalize-model-id';
import { HuggingFaceRepoSibling } from '@/domain/models/huggingface.interface';
import { printLastErrorLines } from '@/utils/logs';

@Injectable()
export class ModelsCliUsecases {
  constructor(
    private readonly modelsUsecases: ModelsUsecases,
    @Inject(InquirerService)
    private readonly inquirerService: InquirerService,
    private readonly fileService: FileManagerService,
  ) {}

  /**
   * Start a model by ID
   * @param modelId
   */
  async startModel(
    modelId: string,
    preset?: string,
  ): Promise<StartModelSuccessDto> {
    const parsedPreset = await this.parsePreset(preset);
    return this.getModelOrStop(modelId)
      .then((model) => ({
        ...model,
        ...parsedPreset,
      }))
      .then((settings) => this.modelsUsecases.startModel(modelId, settings))
      .catch(async (e) => {
        console.error('Model start failed with reason:', e.message);

        printLastErrorLines(await this.fileService.getLogPath());

        console.log(
          'For more information, please check the logs at: %s',
          await this.fileService.getLogPath(),
        );
        process.exit(1);
      });
  }

  /**
   * Stop a model by ID
   * @param modelId
   */
  async stopModel(modelId: string): Promise<void> {
    return this.modelsUsecases.stopModel(modelId).then();
  }

  /**
   * Update a model by ID with new data
   */
  async updateModel(modelId: string, toUpdate: UpdateModelDto) {
    return this.modelsUsecases.update(modelId, toUpdate);
  }

  /**
   * Find a model or abort if not exist
   * @param modelId
   * @returns
   */
  async getModelOrStop(modelId: string): Promise<Model> {
    const model = await this.modelsUsecases.findOne(modelId);
    if (!model) {
      console.debug('Model not found');
      exit(1);
    }
    return model;
  }

  /**
   * List all of the models
   * @returns
   */
  async listAllModels(): Promise<Model[]> {
    return this.modelsUsecases.findAll();
  }

  /**
   * Get a model by ID
   * @param modelId
   * @returns
   */
  async getModel(modelId: string): Promise<Model | null> {
    return this.modelsUsecases.findOne(modelId);
  }

  /**
   * Remove a model, this would also delete model files
   * @param modelId
   * @returns
   */
  async removeModel(modelId: string) {
    await this.getModelOrStop(modelId);
    return this.modelsUsecases.remove(modelId);
  }

  /**
   * Pull model from Model repository (HF, Jan...)
   * @param modelId
   */
  async pullModel(modelId: string) {
    const existingModel = await this.modelsUsecases.findOne(modelId);
    if (isLocalModel(existingModel?.files)) {
      console.error('Model already exists');
      process.exit(1);
    }
    // Checking dependencies

    console.log(
      `${modelId} not found on filesystem.\nDownloading from remote: https://huggingface.co/${modelId.includes('/') ? modelId : 'cortexso'} ...`,
    );
    await this.modelsUsecases.pullModel(modelId, true, (files) => {
      return new Promise<HuggingFaceRepoSibling>(async (resolve) => {
        const listChoices = files
          .filter((e) => e.quantization != null)
          .map((e) => {
            return {
              name: e.quantization,
              value: e.quantization,
            };
          });

        if (listChoices.length > 1) {
          const { quantization } = await this.inquirerService.inquirer.prompt({
            type: 'list',
            name: 'quantization',
            message: 'Select quantization',
            choices: listChoices,
          });
          resolve(
            files
              .filter((e) => !!e.quantization)
              .find((e: any) => e.quantization === quantization) ?? files[0],
          );
        } else {
          resolve(files.find((e) => e.rfilename.includes('.gguf')) ?? files[0]);
        }
      });
    });
  }

  /**
   * Parse preset file
   * @param preset
   * @returns
   */
  private async parsePreset(preset?: string): Promise<object> {
    const presetsFolder = await this.fileService.getPresetsPath();

    if (!existsSync(presetsFolder)) return {};

    const presetFile = readdirSync(presetsFolder).find(
      (file) =>
        file.toLowerCase() === `${preset?.toLowerCase()}.yaml` ||
        file.toLowerCase() === `${preset?.toLocaleLowerCase()}.yml`,
    );
    if (!presetFile) return {};
    const presetPath = join(presetsFolder, presetFile);

    if (!preset || !existsSync(presetPath)) return {};
    return preset
      ? (load(readFileSync(join(presetPath), 'utf-8')) as object)
      : {};
  }
}
