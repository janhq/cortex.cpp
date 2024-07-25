import { Presets, SingleBar } from "cli-progress";
import { Cortex } from "cortexso-node";
import { exit, stdin, stdout } from 'node:process';

export const downloadModelProgress = async (cortex: Cortex, modelId: string) => {
    const response = await cortex.events.downloadEvent();

    const rl = require('readline').createInterface({
      input: stdin,
      output: stdout,
    });

    rl.on('SIGINT', () => {
      console.log('\nStopping download...');
      process.emit('SIGINT');
    });
    process.on('SIGINT', async () => {
      await cortex.models.abortDownload(modelId);
      exit(1);
    });

    const progressBar = new SingleBar({}, Presets.shades_classic);
    progressBar.start(100, 0);

    for await (const stream of response) {
      if (stream.length) {
        const data = stream[0] as any;

        if (data.status === 'downloaded') break;

        let totalBytes = 0;
        let totalTransferred = 0;
        data.children.forEach((child: any) => {
          totalBytes += child.size.total;
          totalTransferred += child.size.transferred;
        });
        progressBar.update(Math.floor((totalTransferred / (totalBytes || 1)) * 100));
      }
    }
    progressBar.stop();
    rl.close();
};