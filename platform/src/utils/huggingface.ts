//// PRIVATE METHODS ////

import {
  HuggingFaceRepoData,
  HuggingFaceRepoSibling,
} from '@/domain/models/huggingface.interface';
import { ModelMetadata } from '@/infrastructure/commanders/types/model-tokenizer.interface';
import {
  AllQuantizations,
  HUGGING_FACE_DOWNLOAD_FILE_MAIN_URL,
  HUGGING_FACE_REPO_MODEL_API_URL,
  HUGGING_FACE_REPO_URL,
  HUGGING_FACE_TREE_REF_URL,
} from '@/infrastructure/constants/huggingface';
import {
  LLAMA_2,
  LLAMA_3,
  LLAMA_3_JINJA,
  OPEN_CHAT_3_5,
  OPEN_CHAT_3_5_JINJA,
  ZEPHYR,
  ZEPHYR_JINJA,
} from '@/infrastructure/constants/prompt-constants';
import axios from 'axios';
import { parseModelHubEngineBranch } from './normalize-model-id';
import { closeSync, openSync, readSync } from 'fs';

// TODO: move this to somewhere else, should be reused by API as well. Maybe in a separate service / provider?
export function guessPromptTemplateFromHuggingFace(jinjaCode?: string): string {
  if (!jinjaCode) {
    console.log('No jinja code provided. Returning default LLAMA_2');
    return LLAMA_2;
  }

  if (typeof jinjaCode !== 'string') {
    console.log(
      `Invalid jinja code provided (type is ${typeof jinjaCode}). Returning default LLAMA_2`,
    );
    return LLAMA_2;
  }

  //TODO: Missing supported templates?
  switch (jinjaCode) {
    case ZEPHYR_JINJA:
      return ZEPHYR;

    case OPEN_CHAT_3_5_JINJA:
      return OPEN_CHAT_3_5;

    case LLAMA_3_JINJA:
      return LLAMA_3;

    default:
      // console.log(
      //   'Unknown jinja code:',
      //   jinjaCode,
      //   'Returning default LLAMA_2',
      // );
      return LLAMA_2;
  }
}

/**
 * Fetch the model data from Jan's repo
 * @param modelId HuggingFace model id. e.g. "llama-3:7b"
 * @returns
 */
export async function fetchJanRepoData(
  modelId: string,
): Promise<HuggingFaceRepoData> {
  const repo = modelId.split(':')[0];
  const tree = await parseModelHubEngineBranch(
    modelId.split(':')[1] ?? (!modelId.includes('/') ? 'main' : ''),
  );
  const url = getRepoModelsUrl(
    `${!modelId.includes('/') ? 'cortexso/' : ''}${repo}`,
    tree,
  );

  const res = await fetch(url);
  const jsonData = await res.json();
  if ('siblings' in jsonData) {
    AllQuantizations.forEach((quantization) => {
      jsonData.siblings.forEach((sibling: HuggingFaceRepoSibling) => {
        if (!sibling.quantization && sibling.rfilename.includes(quantization)) {
          sibling.quantization = quantization;
          sibling.downloadUrl = HUGGING_FACE_DOWNLOAD_FILE_MAIN_URL(
            repo,
            sibling.rfilename,
          );
        }
      });
    });
    return jsonData as HuggingFaceRepoData;
  }

  const response:
    | {
        path: string;
        size: number;
        oid?: string;
      }[]
    | { error: string } = jsonData;

  if ('error' in response && response.error != null) {
    throw new Error(response.error);
  }

  // TODO: Support multiple engines
  const data: HuggingFaceRepoData = {
    siblings: Array.isArray(response)
      ? response.map((e) => {
          return {
            rfilename: e.path,
            downloadUrl: HUGGING_FACE_TREE_REF_URL(repo, tree, e.path),
            fileSize: e.size ?? 0,
            lfs: {
              oid: e.oid,
            },
          };
        })
      : [],
    tags: ['gguf'],
    id: modelId,
    modelId: modelId,
    author: 'cortexso',
    sha: '',
    downloads: 0,
    lastModified: '',
    private: false,
    disabled: false,
    gated: false,
    pipeline_tag: 'text-generation',
    cardData: {},
    createdAt: '',
  };

  AllQuantizations.forEach((quantization) => {
    data.siblings.forEach((sibling: any) => {
      if (!sibling.quantization && sibling.rfilename.includes(quantization)) {
        sibling.quantization = quantization;
      }
    });
  });

  data.modelUrl = url;
  return data;
}

/**
 * Get the URL to fetch the model data from HuggingFace API
 * @param repoId HuggingFace model id. e.g. "janhq/llama3"
 * @param tree The tree ref. e.g. "8b"
 * @returns The URL to fetch the model data from HuggingFace API
 */
export function getRepoModelsUrl(repoId: string, tree?: string): string {
  return `${HUGGING_FACE_REPO_MODEL_API_URL(repoId)}${tree ? `/tree/${tree}` : ''}`;
}

/**
 * Get the model metadata from HuggingFace
 * @param ggufUrl The URL to the GGUF file
 * @returns The model tokenizer
 */
export async function getHFModelMetadata(
  ggufUrl: string,
): Promise<ModelMetadata | undefined> {
  try {
    const { ggufMetadata } = await import('hyllama');
    // Read first 10mb of gguf file
    const fd = openSync(ggufUrl, 'r');
    const buffer = new Uint8Array(10_000_000);
    readSync(fd, buffer, 0, 10_000_000, 0);
    closeSync(fd);

    // Parse metadata and tensor info
    const { metadata } = ggufMetadata(buffer.buffer);

    const index = metadata['tokenizer.ggml.eos_token_id'];
    const hfChatTemplate = metadata['tokenizer.chat_template'];
    const promptTemplate = guessPromptTemplateFromHuggingFace(hfChatTemplate);
    const stopWord: string = metadata['tokenizer.ggml.tokens'][index] ?? '';
    const name = metadata['general.name'];
    const contextLength = metadata['llama.context_length'] ?? 4096;
    const ngl = (metadata['llama.block_count'] ?? 32) + 1;
    const version: number = metadata['version'];

    return {
      contextLength,
      ngl,
      stopWord,
      promptTemplate,
      version,
      name,
    };
  } catch (err) {
    console.log('Failed to get model metadata:', err.message);
    return undefined;
  }
}
