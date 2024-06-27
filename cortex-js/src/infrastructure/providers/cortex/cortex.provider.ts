import { Injectable } from '@nestjs/common';
import { OAIEngineExtension } from '@/domain/abstracts/oai.abstract';
import { PromptTemplate } from '@/domain/models/prompt-template.interface';
import { join } from 'path';
import { Model, ModelSettingParams } from '@/domain/models/model.interface';
import { HttpService } from '@nestjs/axios';
import {
  defaultCortexCppHost,
  defaultCortexCppPort,
} from '@/infrastructure/constants/cortex';
import { readdirSync } from 'node:fs';
import { normalizeModelId } from '@/utils/normalize-model-id';
import { firstValueFrom } from 'rxjs';
import { FileManagerService } from '@/infrastructure/services/file-manager/file-manager.service';

@Injectable()
export default class CortexProvider extends OAIEngineExtension {
  apiUrl = `http://${defaultCortexCppHost}:${defaultCortexCppPort}/inferences/server/chat_completion`;
  name = 'cortex';
  productName = 'Cortex Inference Engine';
  description =
    'This extension enables chat completion API calls using the Cortex engine';
  version = '0.0.1';
  apiKey?: string | undefined;

  private loadModelUrl = `http://${defaultCortexCppHost}:${defaultCortexCppPort}/inferences/server/loadmodel`;
  private unloadModelUrl = `http://${defaultCortexCppHost}:${defaultCortexCppPort}/inferences/server/unloadmodel`;

  constructor(
    protected readonly httpService: HttpService,
    private readonly fileManagerService: FileManagerService,
  ) {
    super(httpService);
  }

  override async loadModel(
    model: Model,
    settings?: ModelSettingParams,
  ): Promise<void> {
    const modelsContainerDir = await this.fileManagerService.getModelsPath();

    let llama_model_path = settings?.llama_model_path;
    if (!llama_model_path) {
      const modelFolderFullPath = join(
        modelsContainerDir,
        normalizeModelId(model.model),
      );
      const ggufFiles = readdirSync(modelFolderFullPath).filter((file) => {
        return file.endsWith('.gguf');
      });

      if (ggufFiles.length === 0) {
        throw new Error('Model binary not found');
      }

      const modelBinaryLocalPath = join(modelFolderFullPath, ggufFiles[0]);
      llama_model_path = modelBinaryLocalPath;
    }

    const cpuThreadCount = 1; // TODO: Math.max(1, nitroResourceProbe.numCpuPhysicalCore);
    const modelSettings = {
      // This is critical and requires real CPU physical core count (or performance core)
      cpu_threads: cpuThreadCount,
      ...model,
      ...settings,
      llama_model_path,
      ...('mmproj' in model.files &&
        model.files.mmproj && {
          mmproj: settings?.mmproj,
        }),
    };

    // Convert settings.prompt_template to system_prompt, user_prompt, ai_prompt
    if (model.prompt_template) {
      const promptTemplate = model.prompt_template;
      const prompt = this.promptTemplateConverter(promptTemplate);
      if (prompt?.error) {
        throw new Error(prompt.error);
      }
      modelSettings.system_prompt = prompt.system_prompt;
      modelSettings.user_prompt = prompt.user_prompt;
      modelSettings.ai_prompt = prompt.ai_prompt;
    }
    return firstValueFrom(
      this.httpService.post(this.loadModelUrl, modelSettings),
    ).then(); // pipe error or void instead of throwing
  }

  override async unloadModel(modelId: string): Promise<void> {
    return firstValueFrom(
      this.httpService.post(this.unloadModelUrl, { model: modelId }),
    ).then(); // pipe error or void instead of throwing
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
