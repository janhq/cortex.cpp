import { Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '../../infrastructure/dtos/chat/create-chat-completion.dto';
import { Response } from 'express';
import { ModelNotFoundException } from 'src/exceptions/model-not-found.exception';
import { ModelSettingNotFoundException } from 'src/exceptions/model-setting-not-found.exception';
import { ModelsUsecases } from '../models/models.usecases';
import { InferenceSettingsUsecases } from '../inference-settings/inference-settings.usecases';
import {
  Model,
  RemoteInferenceEngines,
} from 'src/domain/models/model.interface';

@Injectable()
export class ChatUsecases {
  constructor(
    private readonly inferenceSettingUsecases: InferenceSettingsUsecases,
    private readonly modelUsecases: ModelsUsecases,
  ) {}

  async createChatCompletions(
    createChatDto: CreateChatCompletionDto,
    res: Response,
  ) {
    const { model: modelId } = createChatDto;
    const model = await this.modelUsecases.findOne(modelId);

    if (!model) {
      throw new ModelNotFoundException(modelId);
    }

    if (RemoteInferenceEngines.includes(model.engine)) {
      return this.handleRemoteInference(createChatDto, model, res);
    }

    return {}; // TODO: NamH
  }

  private async handleRemoteInference(
    createChatDto: CreateChatCompletionDto,
    model: Model,
    res: Response,
  ) {
    // getting setting
    const engineSetting = await this.inferenceSettingUsecases.findOne(
      model.engine,
    );

    if (!engineSetting) {
      throw new ModelSettingNotFoundException(model.engine);
    }

    const { settings } = engineSetting;
    let apiKey: string | undefined = undefined;
    let apiUrl: string | undefined = undefined;

    settings.forEach((setting) => {
      if (setting.key.includes('api-key'))
        apiKey = setting.controllerProps.value;
      else if (setting.key.includes('chat-completions-endpoint'))
        apiUrl = setting.controllerProps.value;
    });

    if (!apiKey || !apiUrl) {
      // TODO: NamH create exception
      throw new Error('API Key or API URL not found');
    }

    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      Authorization: `Bearer ${apiKey}`,
      'api-key': apiKey,
    };

    console.log('createChatDto', settings);
    // TODO: openAI only allow 4 stop words
    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const fetch = require('node-fetch');
    const response = await fetch(apiUrl, {
      method: 'POST',
      headers: headers,
      body: JSON.stringify(createChatDto),
    });
    res.writeHead(200, {
      'Content-Type':
        createChatDto.stream === true
          ? 'text/event-stream'
          : 'application/json',
      'Cache-Control': 'no-cache',
      Connection: 'keep-alive',
      'Access-Control-Allow-Origin': '*',
    });
    response.body?.pipe(res);
  }
}
