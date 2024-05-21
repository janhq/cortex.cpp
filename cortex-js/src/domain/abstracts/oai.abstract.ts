import { HttpService } from '@nestjs/axios';
import { EngineExtension } from './engine.abstract';
import stream from 'stream';

export abstract class OAIEngineExtension extends EngineExtension {
  abstract apiUrl: string;

  constructor(protected readonly httpService: HttpService) {
    super();
  }

  override async inferenceStream(
    createChatDto: any,
    headers: Record<string, string>,
  ): Promise<stream.Readable> {
    const response = await this.httpService
      .post(this.apiUrl, createChatDto, {
        headers: {
          'Content-Type': headers['content-type'] ?? 'application/json',
          Authorization: headers['authorization'],
        },
        responseType: 'stream',
      })
      .toPromise();

    if (!response) {
      throw new Error('No response');
    }

    return response.data;
  }

  override async inference(
    createChatDto: any,
    headers: Record<string, string>,
  ): Promise<any> {
    const response = await this.httpService
      .post(this.apiUrl, createChatDto, {
        headers: {
          'Content-Type': headers['content-type'] ?? 'application/json',
          Authorization: headers['authorization'],
        },
      })
      .toPromise();
    if (!response) {
      throw new Error('No response');
    }

    return response.data;
  }
}
