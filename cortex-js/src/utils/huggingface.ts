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
import { gguf } from '@huggingface/gguf';
import axios from 'axios';
import { parseModelHubEngineBranch } from './normalize-model-id';

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
 * Fetches the model data from HuggingFace API
 * @param repoId HuggingFace model id. e.g. "janhq/llama-3"
 * @returns
 */
export async function fetchHuggingFaceRepoData(
  repoId: string,
): Promise<HuggingFaceRepoData> {
  const sanitizedUrl = getRepoModelsUrl(repoId);

  const { data: response } = await axios.get(sanitizedUrl);
  if (response['error'] != null) {
    throw new Error(response['error']);
  }

  const data = response as HuggingFaceRepoData;

  if (data.tags.indexOf('gguf') === -1) {
    throw `${repoId} is not supported. Only GGUF models are supported.`;
  }

  // fetching file sizes
  const url = new URL(sanitizedUrl);
  const paths = url.pathname.split('/').filter((e) => e.trim().length > 0);

  for (let i = 0; i < data.siblings.length; i++) {
    const downloadUrl = HUGGING_FACE_DOWNLOAD_FILE_MAIN_URL(
      [paths[2], paths[3]].join('/'),
      data.siblings[i].rfilename,
    );
    data.siblings[i].downloadUrl = downloadUrl;
  }

  //TODO: Very hacky? Let's say they don't name it properly
  AllQuantizations.forEach((quantization) => {
    data.siblings.forEach((sibling: any) => {
      if (!sibling.quantization && sibling.rfilename.includes(quantization)) {
        sibling.quantization = quantization;
      }
    });
  });

  data.modelUrl = HUGGING_FACE_REPO_URL(paths[2], paths[3]);
  return data;
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
    modelId.split(':')[1] ?? !modelId.includes('/') ? 'default' : '',
  );
  const url = getRepoModelsUrl(
    `${!modelId.includes('/') ? 'cortexhub/' : ''}${repo}`,
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
          };
        })
      : [],
    tags: ['gguf'],
    id: modelId,
    modelId: modelId,
    author: 'cortexhub',
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
    const { metadata } = await gguf(ggufUrl);
    // @ts-expect-error "tokenizer.ggml.eos_token_id"
    const index = metadata['tokenizer.ggml.eos_token_id'];
    // @ts-expect-error "tokenizer.ggml.eos_token_id"
    const hfChatTemplate = metadata['tokenizer.chat_template'];
    const promptTemplate = guessPromptTemplateFromHuggingFace(hfChatTemplate);
    // @ts-expect-error "tokenizer.ggml.tokens"
    const stopWord: string = metadata['tokenizer.ggml.tokens'][index] ?? '';

    const version: number = metadata['version'];
    return {
      stopWord,
      promptTemplate,
      version,
    };
  } catch (err) {
    console.log('Failed to get model metadata:', err.message);
    return undefined;
  }
}
