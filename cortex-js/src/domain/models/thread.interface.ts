import { Assistant } from './assistant.interface';
import { Cortex } from '@cortexso/cortex.js';

export interface ThreadToolResources
  extends Cortex.Beta.Threads.Thread.ToolResources {}

export interface Thread extends Cortex.Beta.Threads.Thread {
  title: string;

  assistants: Assistant[];

  tool_resources: ThreadToolResources | null;
}
