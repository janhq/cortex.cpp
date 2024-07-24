import { Cortex } from 'cortexso-node';

export interface Message extends Cortex.Beta.Threads.Message {}

export type MessageContent = Cortex.Beta.Threads.MessageContent;

export type TextContentBlock = Cortex.Beta.Threads.TextContentBlock;

export interface MessageIncompleteDetails
  extends Cortex.Beta.Threads.Message.IncompleteDetails {}

export interface MessageAttachment
  extends Cortex.Beta.Threads.Message.Attachment {}
