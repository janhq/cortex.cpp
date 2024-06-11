export class DownloadState {
  /**
   * The id of a particular download. Being used to prevent duplication of downloads.
   */
  id: string;

  /**
   * For displaying purposes.
   */
  title: string;

  /**
   * The type of download.
   */
  type: DownloadType;

  /**
   * The status of the download.
   */
  status: DownloadStatus;

  /**
   * Explanation of the error if the download failed.
   */
  error?: string;

  /**
   * The actual downloads. [DownloadState] is just a group to supporting for download multiple files.
   */
  children: DownloadItem[];
}

export enum DownloadStatus {
  Pending = 'pending',
  Downloading = 'downloading',
  Error = 'error',
  Downloaded = 'downloaded',
}

export class DownloadItem {
  /**
   * Filename of the download.
   */
  id: string;

  time: {
    elapsed: number;
    remaining: number;
  };

  size: {
    total: number;
    transferred: number;
  };

  checksum?: string;

  status: DownloadStatus;

  error?: string;

  metadata?: Record<string, unknown>;
}

export interface DownloadStateEvent {
  data: DownloadState[];
}

export enum DownloadType {
  Model = 'model',
  Miscelanous = 'miscelanous',
}
