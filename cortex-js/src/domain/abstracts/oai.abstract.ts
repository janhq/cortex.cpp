/* eslint-disable @typescript-eslint/no-unused-vars */
import { HttpService } from '@nestjs/axios';
import { EngineExtension } from './engine.abstract';

export abstract class OAIEngineExtension extends EngineExtension {
  abstract apiUrl: string;

  constructor(protected readonly httpService: HttpService) {
    super();
  }

  async inference(
    createChatDto: any,
    headers: Record<string, string>,
    res: any,
  ) {
    if (createChatDto.stream === true) {
      const response = await this.httpService
        .post(this.apiUrl, createChatDto, {
          headers: {
            'Content-Type': headers['content-type'] ?? 'application/json',
            Authorization: headers['authorization'],
          },
          responseType: 'stream',
        })
        .toPromise();

      res.writeHead(200, {
        'Content-Type': 'text/event-stream',
        'Cache-Control': 'no-cache',
        Connection: 'keep-alive',
        'Access-Control-Allow-Origin': '*',
      });

      response?.data.pipe(res);
    } else {
      const response = await this.httpService
        .post(this.apiUrl, createChatDto, {
          headers: {
            'Content-Type': headers['content-type'] ?? 'application/json',
            Authorization: headers['authorization'],
          },
        })
        .toPromise();

      res.json(response?.data);
    }
  }

  async loadModel(_loadModel: any): Promise<void> {}
  async unloadModel(_modelId: string): Promise<void> {}
}
