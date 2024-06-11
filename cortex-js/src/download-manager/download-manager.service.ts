import {
  DownloadItem,
  DownloadState,
  DownloadStatus,
  DownloadType,
} from '@/domain/models/download.interface';
import { HttpService } from '@nestjs/axios';
import { Injectable } from '@nestjs/common';
import { EventEmitter2 } from '@nestjs/event-emitter';
import { createWriteStream } from 'node:fs';
import { firstValueFrom } from 'rxjs';

@Injectable()
export class DownloadManagerService {
  private allDownloadStates: DownloadState[] = [];

  constructor(
    private readonly httpService: HttpService,
    private readonly eventEmitter: EventEmitter2,
  ) {
    // start emitting download state each 500ms
    setInterval(() => {
      this.eventEmitter.emit('download.event', this.allDownloadStates);
    }, 500);
  }

  async submitDownloadRequest(
    downloadId: string,
    title: string,
    downloadType: DownloadType,
    urlToDestination: Record<string, string>,
  ) {
    if (
      this.allDownloadStates.find(
        (downloadState) => downloadState.id === downloadId,
      )
    ) {
      return;
    }

    const downloadItems: DownloadItem[] = Object.keys(urlToDestination).map(
      (url) => {
        const destination = urlToDestination[url];
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

    Object.keys(urlToDestination).forEach((url) => {
      const destination = urlToDestination[url];
      this.downloadFile(downloadId, url, destination);
    });
  }

  private async downloadFile(
    downloadId: string,
    url: string,
    destination: string,
  ) {
    const response = await firstValueFrom(
      this.httpService.get(url, {
        responseType: 'stream',
      }),
    );
    // check if response is success
    if (!response) {
      throw new Error('Failed to download model');
    }

    const writer = createWriteStream(destination);
    const totalBytes = response.headers['content-length'];

    // update download state
    // TODO: separte this duplicate code to a function
    const currentDownloadState = this.allDownloadStates.find(
      (downloadState) => downloadState.id === downloadId,
    );
    if (!currentDownloadState) {
      return;
    }
    const downloadItem = currentDownloadState?.children.find(
      (downloadItem) => downloadItem.id === destination,
    );
    if (downloadItem) {
      downloadItem.size.total = totalBytes;
    }

    let transferredBytes = 0;

    writer.on('finish', () => {
      const currentDownloadState = this.allDownloadStates.find(
        (downloadState) => downloadState.id === downloadId,
      );
      if (!currentDownloadState) {
        return;
      }

      // update current child status to downloaded, find by destination as id
      const downloadItem = currentDownloadState?.children.find(
        (downloadItem) => downloadItem.id === destination,
      );
      if (downloadItem) {
        downloadItem.status = DownloadStatus.Downloaded;
      }

      const allChildrenDownloaded = currentDownloadState?.children.every(
        (downloadItem) => downloadItem.status === DownloadStatus.Downloaded,
      );

      if (allChildrenDownloaded) {
        currentDownloadState.status = DownloadStatus.Downloaded;
        // TODO: notify if download success so that client can auto refresh
        // remove download state if all children is downloaded
        this.allDownloadStates = this.allDownloadStates.filter(
          (downloadState) => downloadState.id !== downloadId,
        );
      }
    });

    writer.on('error', (error) => {
      const currentDownloadState = this.allDownloadStates.find(
        (downloadState) => downloadState.id === downloadId,
      );
      if (!currentDownloadState) {
        return;
      }

      const downloadItem = currentDownloadState?.children.find(
        (downloadItem) => downloadItem.id === destination,
      );
      if (downloadItem) {
        downloadItem.status = DownloadStatus.Error;
        downloadItem.error = error.message;
      }

      currentDownloadState.status = DownloadStatus.Error;
      currentDownloadState.error = error.message;

      // TODO: notify if download error
      // remove download state if all children is downloaded
      this.allDownloadStates = this.allDownloadStates.filter(
        (downloadState) => downloadState.id !== downloadId,
      );
    });

    response.data.on('data', (chunk: any) => {
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
      }
    });

    response.data.pipe(writer);
  }
}
