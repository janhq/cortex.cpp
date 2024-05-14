import { Injectable } from '@nestjs/common';
import { OAIEngineExtension } from '@/domain/abstracts/oai.abstract';
import { PromptTemplate } from '@/domain/models/prompt-template.interface';
import { basename, join, resolve } from 'path';
import { Model } from '@/domain/models/model.interface';
import { ConfigService } from '@nestjs/config';
import { HttpService } from '@nestjs/axios';

/**
 * A class that implements the InferenceExtension interface from the @janhq/core package.
 * The class provides methods for initializing and stopping a model, and for making inference requests.
 * It also subscribes to events emitted by the @janhq/core package and handles new message requests.
 */
const LOCAL_HOST = '127.0.0.1';
const NITRO_DEFAULT_PORT = 3928;
const NITRO_HTTP_SERVER_URL = `http://${LOCAL_HOST}:${NITRO_DEFAULT_PORT}`;
const LOAD_MODEL_URL = `${NITRO_HTTP_SERVER_URL}/inferences/server/loadmodel`;
const UNLOAD_MODEL_URL = `${NITRO_HTTP_SERVER_URL}/inferences/server/unloadmodel`;

@Injectable()
export default class CortexProvider extends OAIEngineExtension {
  provider: string = 'cortex';
  apiUrl = 'http://127.0.0.1:3928/inferences/server/chat_completion';

  constructor(
    private readonly configService: ConfigService,
    protected readonly httpService: HttpService,
  ) {
    super(httpService);
  }

  override async loadModel(model: Model): Promise<void> {
    const modelsContainerDir =
      this.configService.get<string>('CORTEX_MODELS_DIR') ??
      resolve('./models');

    const modelFolderFullPath = join(modelsContainerDir, model.id);
    //TODO: recheck this
    const modelBinaryLocalPath = join(
      modelFolderFullPath,
      basename(model.sources[0].url),
    );

    // TODO: NamH check if the binary is there

    const cpuThreadCount = 1; // TODO: NamH Math.max(1, nitroResourceProbe.numCpuPhysicalCore);
    const modelSettings = {
      // This is critical and requires real CPU physical core count (or performance core)
      model: model.id,
      cpu_threads: cpuThreadCount,
      ...model.settings,
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

    await this.httpService.post(LOAD_MODEL_URL, modelSettings).toPromise();
  }

  override async unloadModel(modelId: string): Promise<void> {
    await this.httpService
      .post(UNLOAD_MODEL_URL, { model: modelId })
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
