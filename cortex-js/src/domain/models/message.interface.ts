import {
  Message as OpenAiMessage,
  MessageContent as OpenAiMessageContent,
  TextContentBlock as OpenAiTextContentBlock,
} from 'openai/resources/beta/threads/messages';

export interface Message extends OpenAiMessage {}

export type MessageContent = OpenAiMessageContent;

export type TextContentBlock = OpenAiTextContentBlock;

export interface MessageIncompleteDetails
  extends OpenAiMessage.IncompleteDetails {}

export interface MessageAttachment extends OpenAiMessage.Attachment {}
