import { CORTEX_JS_STOP_API_SERVER_URL } from '@/infrastructure/constants/cortex';
import { SubCommand } from 'nest-commander';
import { BaseCommand } from '../base.command';

@SubCommand({
  name: 'stop',
  description: 'Stop the API server',
})
export class ServeStopCommand extends BaseCommand {
  async runCommand(): Promise<void> {
    return this.stopServer().then(() => console.log('API server stopped'));
  }

  private async stopServer() {
    return fetch(CORTEX_JS_STOP_API_SERVER_URL(), {
      method: 'DELETE',
    }).catch(() => {});
  }
}
