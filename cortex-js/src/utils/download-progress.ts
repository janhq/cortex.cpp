import { Presets, SingleBar } from "cli-progress";
import { Cortex } from "@cortexso/cortex.js";
import { exit, stdin, stdout } from 'node:process';
import { DownloadType } from "@/domain/models/download.interface";

export const downloadProgress = async (cortex: Cortex, downloadId?: string, downloadType?: DownloadType) => {
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
      if (downloadId){
        await cortex.models.abortDownload(downloadId);
      }
      exit(1);
    });

    const progressBar = new SingleBar({}, Presets.shades_classic);
    progressBar.start(100, 0);

    for await (const stream of response) {
      if (stream.length) {
        const data = stream[0] as any;
        if(downloadId && data.id !== downloadId) continue;
        if (downloadType && data.type !== downloadType) continue;
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