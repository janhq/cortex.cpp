import { Assistant as OpenAiAssistant } from 'cortexso-node/resources/beta/assistants';
import { AssistantResponseFormatOption as OpenAIAssistantResponseFormatOption } from 'cortexso-node/resources/beta/threads/threads';

export interface Assistant extends OpenAiAssistant {
  avatar?: string;
}

export type AssistantResponseFormatOption = OpenAIAssistantResponseFormatOption;

export interface AssistantToolResources extends OpenAiAssistant.ToolResources {}
