import { Injectable } from '@nestjs/common';

@Injectable()
export class LogService {
  log(...args: any[]): void {
    console.log(...args);
  }
}
