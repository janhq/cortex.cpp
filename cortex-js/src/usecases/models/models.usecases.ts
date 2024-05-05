import { Inject, Injectable } from '@nestjs/common';
import { CreateModelDto } from '../../infrastructure/dtos/models/create-model.dto';
import { UpdateModelDto } from '../../infrastructure/dtos/models/update-model.dto';
import { ModelEntity } from '../../infrastructure/entities/model.entity';
import { Repository } from 'typeorm';
import { ModelNotFoundException } from 'src/exceptions/model-not-found.exception';
import { CortexService } from 'src/cortex/cortex.service';
import { join } from 'path';
import {
  Model,
  NitroModelSettings,
  RemoteInferenceEngines,
} from 'src/domain/models/model.interface';
import { LoadModelDto } from 'src/infrastructure/dtos/models/load-model.dto';
import { LoadModelSuccessDto } from 'src/infrastructure/dtos/models/load-model-success.dto';
import { PromptTemplate } from 'src/domain/models/message.interface';

@Injectable()
export class ModelsUsecases {
  constructor(
    @Inject('MODEL_REPOSITORY')
    private readonly modelRepository: Repository<ModelEntity>,
    private readonly cortexService: CortexService,
  ) {}

  create(createModelDto: CreateModelDto) {
    const model: Model = {
      ...createModelDto,
      object: 'model',
      created: Date.now(),
    };

    this.modelRepository.insert(model);
  }

  async findAll(): Promise<Model[]> {
    return this.modelRepository.find();
  }

  async findOne(id: string) {
    return this.modelRepository.findOne({
      where: {
        id,
      },
    });
  }

  update(id: string, updateModelDto: UpdateModelDto) {
    return this.modelRepository.update(id, updateModelDto);
  }

  async remove(id: string) {
    return this.modelRepository.delete(id);
  }

  async loadModel(loadModelDto: LoadModelDto): Promise<LoadModelSuccessDto> {
    const LOCAL_HOST = '127.0.0.1';
    const NITRO_DEFAULT_PORT = 3928;
    const NITRO_HTTP_SERVER_URL = `http://${LOCAL_HOST}:${NITRO_DEFAULT_PORT}`;
    const url = `${NITRO_HTTP_SERVER_URL}/inferences/llamacpp/loadmodel`;

    const model = await this.findOne(loadModelDto.modelId);
    if (!model) {
      throw new ModelNotFoundException(loadModelDto.modelId);
    }

    if (RemoteInferenceEngines.includes(model.engine)) {
      return {
        message: 'Model loaded successfully',
        modelId: model.id,
      };
    }

    // TODO: NamH fix this, this only support import local model
    const modelBinaryLocalPath = model.sources[0].url;
    const modelFolderFullPath = join(modelBinaryLocalPath, '..');

    // TODO: NamH check if the binary is there

    const cpuThreadCount = 1; // TODO: NamH Math.max(1, nitroResourceProbe.numCpuPhysicalCore);
    const nitroModelSettings: NitroModelSettings = {
      // This is critical and requires real CPU physical core count (or performance core)
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
      nitroModelSettings.system_prompt = prompt.system_prompt;
      nitroModelSettings.user_prompt = prompt.user_prompt;
      nitroModelSettings.ai_prompt = prompt.ai_prompt;
    }

    // TODO: NamH check if we need to stop model first?

    // spawn cortex process
    await this.cortexService.startCortex(LOCAL_HOST, `${NITRO_DEFAULT_PORT}`);

    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const fetch = require('node-fetch');
    const response = await fetch(url, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(nitroModelSettings),
    });

    if (response.status !== 200) {
      return {
        message: 'Model failed to load',
        modelId: model.id,
      };
    } else {
      return {
        message: 'Model loaded successfully',
        modelId: model.id,
      };
    }
  }

  promptTemplateConverter = (promptTemplate: string): PromptTemplate => {
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
