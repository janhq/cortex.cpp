import { Injectable } from '@nestjs/common';
import { AsyncLocalStorage } from 'async_hooks';

@Injectable()
export class ContextService {
  private readonly localStorage = new AsyncLocalStorage();

  init<T>(callback: () => T): T {
    return this.localStorage.run(new Map(), callback);
  }

  get<T>(key: string): T | null {
    const context = this.localStorage.getStore() as Map<string, T>;
    return context?.get(key) ?? null;
  }

  set<T>(key: string, value: T): void {
    const context = this.localStorage.getStore() as Map<string, T>;
    context?.set(key, value);
  }
}
