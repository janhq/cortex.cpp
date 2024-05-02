import { Injectable } from '@nestjs/common';
import { CreateChatCompletionDto } from '../../infrastructure/dtos/chat/create-chat-completion.dto';
import { Response } from 'express';

@Injectable()
export class ChatUsecases {
  constructor() {}

  async createChatCompletions(
    createChatDto: CreateChatCompletionDto,
    res: Response,
  ) {
    const apiKey = '';
    const apiUrl = 'https://api.groq.com/openai/v1/chat/completions';

    const headers: Record<string, any> = {
      'Content-Type': 'application/json',
      Authorization: `Bearer ${apiKey}`,
      'api-key': apiKey,
    };

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
