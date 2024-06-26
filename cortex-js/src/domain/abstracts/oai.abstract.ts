import { HttpService } from '@nestjs/axios';
import { EngineExtension } from './engine.abstract';
import stream from 'stream';
import { firstValueFrom } from 'rxjs';

export abstract class OAIEngineExtension extends EngineExtension {
  abstract apiUrl: string;

  constructor(protected readonly httpService: HttpService) {
    super();
  }

  override onLoad(): void {}

  override async inference(
    createChatDto: any,
    headers: Record<string, string>,
  ): Promise<stream.Readable | any> {
    const { stream } = createChatDto;
    const response = await firstValueFrom(
      this.httpService.post(this.apiUrl, createChatDto, {
        headers: {
          'Content-Type': headers['content-type'] ?? 'application/json',
          Authorization: headers['authorization'],
        },
        responseType: stream ? 'stream' : 'json',
      }),
    );
    if (!response) {
      throw new Error('No response');
    }

    return response.data;
  }
}
