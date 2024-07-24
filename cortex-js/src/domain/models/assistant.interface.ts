import { Cortex } from 'cortexso-node';
// TODO: Why we need to alias these?

export interface Assistant extends Cortex.Beta.Assistant {
  avatar?: string;
}

export type AssistantResponseFormatOption =
  Cortex.Beta.Threads.AssistantResponseFormatOption;

export interface AssistantToolResources
  extends Cortex.Beta.Assistant.ToolResources {}
