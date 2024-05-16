import { Injectable } from '@nestjs/common';
import { OAIEngineExtension } from '@/domain/abstracts/oai.abstract';
import { PromptTemplate } from '@/domain/models/prompt-template.interface';
import { join, resolve } from 'path';
import { Model, ModelSettingParams } from '@/domain/models/model.interface';
import { HttpService } from '@nestjs/axios';
import { defaultCortexCppHost, defaultCortexCppPort } from 'constant';
import { readdirSync } from 'node:fs';

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
@Injectable()
export default class CortexProvider extends OAIEngineExtension {
  provider: string = 'cortex';
  apiUrl = `http://${defaultCortexCppHost}:${defaultCortexCppPort}/inferences/server/chat_completion`;

  private loadModelUrl = `http://${defaultCortexCppHost}:${defaultCortexCppPort}/inferences/server/loadmodel`;
  private unloadModelUrl = `http://${defaultCortexCppHost}:${defaultCortexCppPort}/inferences/server/unloadmodel`;

  constructor(protected readonly httpService: HttpService) {
    super(httpService);
  }

  modelDir = () => resolve(__dirname, `../../../models`);

  override async loadModel(
    model: Model,
    settings?: ModelSettingParams,
  ): Promise<void> {
    const modelsContainerDir = this.modelDir();

    const modelFolderFullPath = join(modelsContainerDir, model.id);
    const ggufFiles = readdirSync(modelFolderFullPath).filter((file) => {
      return file.endsWith('.gguf');
    });

    if (ggufFiles.length === 0) {
      throw new Error('Model binary not found');
    }

    const modelBinaryLocalPath = join(modelFolderFullPath, ggufFiles[0]);

    const cpuThreadCount = 1; // TODO: Math.max(1, nitroResourceProbe.numCpuPhysicalCore);
    const modelSettings = {
      // This is critical and requires real CPU physical core count (or performance core)
      model: model.id,
      cpu_threads: cpuThreadCount,
      ...model.settings,
      ...settings,
      llama_model_path: modelBinaryLocalPath,
      ...(model.settings.mmproj && {
        mmproj: join(modelFolderFullPath, model.settings.mmproj),
      }),
    };

    // Convert settings.prompt_template to system_prompt, user_prompt, ai_prompt
    if (model.settings.prompt_template) {
      const promptTemplate = model.settings.prompt_template;
      const prompt = this.promptTemplateConverter(promptTemplate);
      if (prompt?.error) {
        throw new Error(prompt.error);
      }
      modelSettings.system_prompt = prompt.system_prompt;
      modelSettings.user_prompt = prompt.user_prompt;
      modelSettings.ai_prompt = prompt.ai_prompt;
    }

    await this.httpService.post(this.loadModelUrl, modelSettings).toPromise();
  }

  override async unloadModel(modelId: string): Promise<void> {
    await this.httpService
      .post(this.unloadModelUrl, { model: modelId })
      .toPromise();
  }

  private readonly promptTemplateConverter = (
    promptTemplate: string,
  ): PromptTemplate => {
    // Split the string using the markers
    const systemMarker = '{system_message}';
    const promptMarker = '{prompt}';

    if (
      promptTemplate.includes(systemMarker) &&
      promptTemplate.includes(promptMarker)
    ) {
      // Find the indices of the markers
      const systemIndex = promptTemplate.indexOf(systemMarker);
      const promptIndex = promptTemplate.indexOf(promptMarker);

      // Extract the parts of the string
      const system_prompt = promptTemplate.substring(0, systemIndex);
      const user_prompt = promptTemplate.substring(
        systemIndex + systemMarker.length,
        promptIndex,
      );
      const ai_prompt = promptTemplate.substring(
        promptIndex + promptMarker.length,
      );

      // Return the split parts
      return { system_prompt, user_prompt, ai_prompt };
    } else if (promptTemplate.includes(promptMarker)) {
      // Extract the parts of the string for the case where only promptMarker is present
      const promptIndex = promptTemplate.indexOf(promptMarker);
      const user_prompt = promptTemplate.substring(0, promptIndex);
      const ai_prompt = promptTemplate.substring(
        promptIndex + promptMarker.length,
      );

      // Return the split parts
      return { user_prompt, ai_prompt };
    }

    // Return an error if none of the conditions are met
    return { error: 'Cannot split prompt template' };
  };
}
