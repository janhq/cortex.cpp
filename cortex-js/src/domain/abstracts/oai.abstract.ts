import { HttpService } from '@nestjs/axios';
import { EngineExtension } from './engine.abstract';
import { stdout } from 'process';

export type ChatStreamEvent = {
  type: 'data' | 'error' | 'end';
  data?: any;
  error?: any;
};

export abstract class OAIEngineExtension extends EngineExtension {
  abstract apiUrl: string;

  constructor(protected readonly httpService: HttpService) {
    super();
  }

  inference(
    createChatDto: any,
    headers: Record<string, string>,
    writableStream: WritableStream<ChatStreamEvent>,
    res?: any,
  ) {
    if (createChatDto.stream === true) {
      if (res) {
        res.writeHead(200, {
          'Content-Type': 'text/event-stream',
          'Cache-Control': 'no-cache',
          Connection: 'keep-alive',
          'Access-Control-Allow-Origin': '*',
        });
        this.httpService
          .post(this.apiUrl, createChatDto, {
            headers: {
              'Content-Type': headers['content-type'] ?? 'application/json',
              Authorization: headers['authorization'],
            },
            responseType: 'stream',
          })
          .toPromise()
          .then((response) => {
            response?.data.pipe(res);
          });
      } else {
        const decoder = new TextDecoder('utf-8');
        const defaultWriter = writableStream.getWriter();
        defaultWriter.ready.then(() => {
          this.httpService
            .post(this.apiUrl, createChatDto, {
              headers: {
                'Content-Type': headers['content-type'] ?? 'application/json',
                Authorization: headers['authorization'],
              },
              responseType: 'stream',
            })
            .subscribe({
              next: (response) => {
                response.data.on('data', (chunk: any) => {
                  let content = '';
                  const text = decoder.decode(chunk);
                  const lines = text.trim().split('\n');
                  let cachedLines = '';
                  for (const line of lines) {
                    try {
                      const toParse = cachedLines + line;
                      if (!line.includes('data: [DONE]')) {
                        const data = JSON.parse(toParse.replace('data: ', ''));
                        content += data.choices[0]?.delta?.content ?? '';

                        if (content.startsWith('assistant: ')) {
                          content = content.replace('assistant: ', '');
                        }

                        if (content !== '') {
                          defaultWriter.write({
                            type: 'data',
                            data: content,
                          });
                        }
                      }
                    } catch {
                      cachedLines = line;
                    }
                  }
                });

                response.data.on('error', (error: any) => {
                  defaultWriter.write({
                    type: 'error',
                    error,
                  });
                });

                response.data.on('end', () => {
                  // stdout.write('Stream end');
                  defaultWriter.write({
                    type: 'end',
                  });
                });
              },

              error: (error) => {
                stdout.write('Stream error: ' + error);
              },
            });
        });
      }
    } else {
      const defaultWriter = writableStream.getWriter();
      defaultWriter.ready.then(() => {
        this.httpService
          .post(this.apiUrl, createChatDto, {
            headers: {
              'Content-Type': headers['content-type'] ?? 'application/json',
              Authorization: headers['authorization'],
            },
          })
          .toPromise()
          .then((response) => {
            defaultWriter.write({
              type: 'data',
              data: response?.data,
            });
          })
          .catch((error: any) => {
            defaultWriter.write({
              type: 'error',
              error,
            });
          });
      });
    }
  }
}
