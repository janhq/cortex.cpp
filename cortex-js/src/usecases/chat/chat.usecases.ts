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
import { InvalidApiUrlException } from 'src/chat/exception/invalid-api-url.exception';

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

    return this.handleLocalInference(createChatDto, res);
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
    let apiKey = '';
    let apiUrl: string | undefined = undefined;
    let stopWordCount: number | undefined = undefined;

    settings.forEach((setting) => {
      if (setting.key.includes('api-key'))
        apiKey = setting.controllerProps.value;
      else if (setting.key.includes('chat-completions-endpoint'))
        apiUrl = setting.controllerProps.value;
      else if (setting.key.includes('stop-word-count')) {
        if (+setting.controllerProps.value) {
          stopWordCount = +setting.controllerProps.value;
        }
      }
    });

    if (!apiUrl) {
      throw new InvalidApiUrlException(model.id, apiUrl);
    }

    if (stopWordCount) {
      createChatDto.stop = createChatDto.stop.slice(0, stopWordCount);
    }

    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      Authorization: `Bearer ${apiKey}`,
      'api-key': apiKey,
    };

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

  private async handleLocalInference(
    createChatDto: CreateChatCompletionDto,
    res: Response,
  ) {
    const apiUrl = 'http://127.0.0.1:3928/inferences/llamacpp/chat_completion';
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
    };

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
