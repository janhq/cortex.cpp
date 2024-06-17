import { CORTEX_JS_STOP_API_SERVER_URL } from '@/infrastructure/constants/cortex';
import { CommandRunner, SubCommand } from 'nest-commander';

@SubCommand({
  name: 'stop',
  description: 'Stop the API server',
})
export class ServeStopCommand extends CommandRunner {
  async run(): Promise<void> {
    return this.stopServer().then(() => console.log('API server stopped'));
  }

  private async stopServer() {
    return fetch(CORTEX_JS_STOP_API_SERVER_URL(), {
      method: 'DELETE',
    }).catch(() => {});
  }
}
