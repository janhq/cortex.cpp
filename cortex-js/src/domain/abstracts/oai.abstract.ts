import { EngineExtension } from './engine.abstract';

export abstract class OAIEngineExtension extends EngineExtension {
  abstract apiUrl: string;

  async inference(
    createChatDto: any,
    headers: Record<string, string>,
    res: any,
  ) {
    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const fetch = require('node-fetch');
    const response = await fetch(this.apiUrl, {
      method: 'POST',
      headers: {
        'Content-Type': headers['content-type'] ?? 'application/json',
        Authorization: headers['authorization'],
      },
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

    response.body.pipe(res);
  }

  // eslint-disable-next-line @typescript-eslint/no-unused-vars
  async loadModel(loadModel: any): Promise<void> {}
}
