import { HttpService } from '@nestjs/axios';
import { EngineExtension } from './engine.abstract';
import stream, { Transform } from 'stream';
import { firstValueFrom } from 'rxjs';
import _ from 'lodash';

export abstract class OAIEngineExtension extends EngineExtension {
  abstract apiUrl: string;
  abstract apiKey?: string;

  constructor(protected readonly httpService: HttpService) {
    super();
  }

  override onLoad(): void {}

  override async inference(
    createChatDto: any,
    headers: Record<string, string>,
  ): Promise<stream.Readable | any> {
    const payload = this.transformPayload
      ? this.transformPayload(createChatDto)
      : createChatDto;
    const { stream: isStream } = payload;
    const additionalHeaders = _.omit(headers, [
      'content-type',
      'authorization',
      'content-length',
    ]);
    const response = await firstValueFrom(
      this.httpService.post(this.apiUrl, payload, {
        headers: {
          'Content-Type': headers['content-type'] ?? 'application/json',
          Authorization: this.apiKey
            ? `Bearer ${this.apiKey}`
            : headers['authorization'],
          ...additionalHeaders,
        },
        responseType: isStream ? 'stream' : 'json',
      }),
    );

    if (!response) {
      throw new Error('No response');
    }
    if (!this.transformResponse) {
      return response.data;
    }
    if (isStream) {
      const transformResponse = this.transformResponse.bind(this);
      const lineStream = new Transform({
        transform(chunk, encoding, callback) {
          const lines = chunk.toString().split('\n');
          const transformedLines = [];
          for (const line of lines) {
            if (line.trim().length > 0) {
              const transformedLine = transformResponse(line, true);
              if (transformedLine) {
                transformedLines.push(`data: ${transformedLine}\n\n`);
              }
            }
          }
          callback(null, transformedLines.join(''));
        },
      });
      return response.data.pipe(lineStream);
    }
    return this.transformResponse(response.data, false);
  }
}
