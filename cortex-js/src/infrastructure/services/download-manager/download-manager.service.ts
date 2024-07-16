import {
  DownloadItem,
  DownloadState,
  DownloadStatus,
  DownloadType,
} from '@/domain/models/download.interface';
import { HttpService } from '@nestjs/axios';
import { Injectable } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { Presets, SingleBar } from 'cli-progress';
import { createWriteStream } from 'node:fs';
import { basename } from 'node:path';
import { firstValueFrom } from 'rxjs';
import crypto from 'crypto';

@Injectable()
export class DownloadManagerService {
  private allDownloadStates: DownloadState[] = [];
  private abortControllers: Record<string, Record<string, AbortController>> =
    {};

  constructor(
    private readonly httpService: HttpService,
    private readonly eventEmitter: EventEmitter2,
  ) {}

  async abortDownload(downloadId: string) {
    if (!this.abortControllers[downloadId]) {
      return;
    }
    Object.keys(this.abortControllers[downloadId]).forEach((destination) => {
      this.abortControllers[downloadId][destination].abort();
    });
    delete this.abortControllers[downloadId];
    this.allDownloadStates = this.allDownloadStates.filter(
      (downloadState) => downloadState.id !== downloadId,
    );
    this.eventEmitter.emit('download.event.aborted', this.allDownloadStates);
  }

  async submitDownloadRequest(
    downloadId: string,
    title: string,
    downloadType: DownloadType,
    urlToDestination: Record<
      string,
      {
        destination: string;
        checksum?: string;
      }
    >,
    finishedCallback?: () => Promise<void>,
    inSequence: boolean = true,
  ) {
    if (
      this.allDownloadStates.find(
        (downloadState) => downloadState.id === downloadId,
      )
    ) {
      return;
    }
    console.log('asdfsdf', urlToDestination);
    const downloadItems: DownloadItem[] = Object.keys(urlToDestination).map(
      (url) => {
        const { destination, checksum } = urlToDestination[url];
        const downloadItem: DownloadItem = {
          id: destination,
          time: {
            elapsed: 0,
            remaining: 0,
          },
          size: {
            total: 0,
            transferred: 0,
          },
          status: DownloadStatus.Downloading,
          checksum,
        };

        return downloadItem;
      },
    );

    const downloadState: DownloadState = {
      id: downloadId,
      title: title,
      type: downloadType,
      status: DownloadStatus.Downloading,
      children: downloadItems,
    };

    this.allDownloadStates.push(downloadState);
    this.abortControllers[downloadId] = {};

    const callBack = async () => {
      // Await post processing callback
      await finishedCallback?.();

      // Finished - update the current downloading states
      delete this.abortControllers[downloadId];
      const currentDownloadState = this.allDownloadStates.find(
        (downloadState) => downloadState.id === downloadId,
      );
      if (currentDownloadState) {
        currentDownloadState.status = DownloadStatus.Downloaded;
        // remove download state if all children is downloaded
        this.allDownloadStates = this.allDownloadStates.filter(
          (downloadState) => downloadState.id !== downloadId,
        );
      }
      this.eventEmitter.emit('download.event', this.allDownloadStates);
    };
    if (!inSequence) {
      return Promise.all(
        Object.keys(urlToDestination).map((url) => {
          const { destination, checksum } = urlToDestination[url];
          return this.downloadFile(downloadId, url, destination, checksum);
        }),
      ).then(callBack);
    } else {
      // Download model file in sequence
      for (const url of Object.keys(urlToDestination)) {
        const { destination, checksum } = urlToDestination[url];
        await this.downloadFile(downloadId, url, destination, checksum);
      }
      return callBack();
    }
  }

  private async downloadFile(
    downloadId: string,
    url: string,
    destination: string,
    checksum?: string,
  ) {
    console.log('Downloading', {
      downloadId,
      url,
      destination,
      checksum,
    });
    const controller = new AbortController();
    // adding to abort controllers
    this.abortControllers[downloadId][destination] = controller;
    return new Promise<void>(async (resolve) => {
      const response = await firstValueFrom(
        this.httpService.get(url, {
          responseType: 'stream',
          signal: controller.signal,
        }),
      );

      // check if response is success
      if (!response) {
        throw new Error('Failed to download model');
      }

      const writer = createWriteStream(destination);
      const hash = crypto.createHash('sha256');
      const totalBytes = Number(response.headers['content-length']);

      // update download state
      const currentDownloadState = this.allDownloadStates.find(
        (downloadState) => downloadState.id === downloadId,
      );
      if (!currentDownloadState) {
        resolve();
        return;
      }
      const downloadItem = currentDownloadState?.children.find(
        (downloadItem) => downloadItem.id === destination,
      );
      if (downloadItem) {
        downloadItem.size.total = totalBytes;
      }

      console.log('Downloading', basename(destination));

      let transferredBytes = 0;
      const bar = new SingleBar({}, Presets.shades_classic);
      bar.start(100, 0);

      writer.on('finish', () => {
        try {
          // delete the abort controller
          delete this.abortControllers[downloadId][destination];
          const currentDownloadState = this.allDownloadStates.find(
            (downloadState) => downloadState.id === downloadId,
          );
          if (!currentDownloadState) return;

          // update current child status to downloaded, find by destination as id
          const downloadItem = currentDownloadState?.children.find(
            (downloadItem) => downloadItem.id === destination,
          );
          const isFileBroken = checksum && checksum === hash.digest('hex');
          if (downloadItem) {
            downloadItem.status = isFileBroken
              ? DownloadStatus.Error
              : DownloadStatus.Downloaded;
            if (isFileBroken) {
              downloadItem.error = 'Checksum is not matched';
            }
          }
          if (isFileBroken) {
            currentDownloadState.status = DownloadStatus.Error;
            currentDownloadState.error = 'Checksum is not matched';
          }

          this.eventEmitter.emit('download.event', this.allDownloadStates);
        } finally {
          bar.stop();
          resolve();
        }
      });

      writer.on('error', (error) => {
        try {
          delete this.abortControllers[downloadId][destination];
          const currentDownloadState = this.allDownloadStates.find(
            (downloadState) => downloadState.id === downloadId,
          );
          if (!currentDownloadState) return;

          const downloadItem = currentDownloadState?.children.find(
            (downloadItem) => downloadItem.id === destination,
          );
          if (downloadItem) {
            downloadItem.status = DownloadStatus.Error;
            downloadItem.error = error.message;
          }

          currentDownloadState.status = DownloadStatus.Error;
          currentDownloadState.error = error.message;

          // remove download state if all children is downloaded
          this.allDownloadStates = this.allDownloadStates.filter(
            (downloadState) => downloadState.id !== downloadId,
          );
          this.eventEmitter.emit('download.event', this.allDownloadStates);
        } finally {
          bar.stop();
          resolve();
        }
      });

      response.data.on('data', (chunk: any) => {
        hash.update(chunk);
        transferredBytes += chunk.length;

        const currentDownloadState = this.allDownloadStates.find(
          (downloadState) => downloadState.id === downloadId,
        );
        if (!currentDownloadState) return;

        const downloadItem = currentDownloadState?.children.find(
          (downloadItem) => downloadItem.id === destination,
        );
        if (downloadItem) {
          downloadItem.size.transferred = transferredBytes;
          bar.update(Math.floor((transferredBytes / totalBytes) * 100));
        }
        this.eventEmitter.emit('download.event', this.allDownloadStates);
      });

      response.data.pipe(writer);
    });
  }

  getDownloadStates() {
    return this.allDownloadStates;
  }
}
