import { RagExtension } from '@/domain/abstracts/rag.extension.abstract';
import {
  defaultCortexJsHost,
  defaultCortexJsPort,
} from '@/infrastructure/constants/cortex';

import { ConfigsUsecases } from '@/usecases/configs/configs.usecase';
import { EventEmitter2 } from '@nestjs/event-emitter';
/**
 * Client parameters
 * These are the parameters that are used to initialize the OpenAIEmbedding and OpenAI clients
 * */
process.env.OPENAI_API_KEY = 'cortex';

const clientParams = {
  timeout: 20000,
  maxRetries: 1,
  apiKey: 'cortex',
  additionalSessionOptions: {
    baseURL: `http://${defaultCortexJsHost}:${defaultCortexJsPort}/v1`,
  },
};
/**
 * LlamaIndexRagExtension
 */

export default class LlamaIndexRagExtension extends RagExtension {
  // Extension name
  name: string = 'llamaindex';
  // Embedding model to use
  model: string = 'cortexso/nomic-embed-text-v1';
  // Other metadata
  description?: string | undefined =
    'LlamaIndex is a framework for building context-augmented generative AI applications with LLMs';
  version?: '0.5.1';
  productName?: string | undefined = 'LlamaIndex';

  // Constructor
  constructor(
    protected readonly configsUsecases: ConfigsUsecases,
    protected readonly eventEmitter: EventEmitter2,
  ) {
    super();
  }

  // On extension load, start the cortex and model
  async onLoad() {
    const configs = (await this.configsUsecases.getGroupConfigs(
      this.name,
    )) as unknown as { model: string };
    if (configs?.model) this.model = configs.model;
  }

  /**
   * split documents, get embeddings, and build index
   * @param files
   * @param storePath
   */
  async fromFiles(
    files: string[],
    storePath: string,
    vectorDatabase: string,
  ): Promise<void> {
    const documents: any[] = [];

    const {
      SimpleDirectoryReader,
      storageContextFromDefaults,
      serviceContextFromDefaults,
      OpenAIEmbedding,
      VectorStoreIndex,
    } = await import('llamaindex');
    for (const file of files) {
      const reader = new SimpleDirectoryReader();
      const data = await reader.loadData({
        directoryPath: file,
      });
      documents.push(...data);
    }

    // Vector Store handler start
    // Handle the vector store initialization here
    let vectorStore: any;
    // TODO: Map supported vector database handlers hgere
    // Vector Store handler end

    const storageContext = await storageContextFromDefaults({
      persistDir: storePath,
      vectorStore,
    });

    const serviceContext = serviceContextFromDefaults({
      embedModel: new OpenAIEmbedding({ ...clientParams, model: this.model }),
    });

    await VectorStoreIndex.fromDocuments(documents, {
      serviceContext,
      storageContext,
    });
  }

  /**
   * Search the index store
   * @param query
   * @param storePath
   * @returns
   */
  async getResponse(
    query: string,
    llmModel: string,
    storePath: string,
    similarityTopK: number = 4,
  ): Promise<AsyncIterable<string>> {
    const {
      serviceContextFromDefaults,
      storageContextFromDefaults,
      OpenAIEmbedding,
      OpenAI,
      VectorStoreIndex,
    } = await import('llamaindex');
    const storageContext = await storageContextFromDefaults({
      persistDir: storePath,
    });

    const serviceContext = serviceContextFromDefaults({
      embedModel: new OpenAIEmbedding({ ...clientParams, model: this.model }),
      llm: new OpenAI({ ...clientParams, model: llmModel }),
    });

    const loadedIndex = await VectorStoreIndex.init({
      serviceContext,
      storageContext,
    });

    const queryEngine = loadedIndex.asQueryEngine({
      similarityTopK,
    });

    const response = await queryEngine.query({
      query,
      stream: true,
    });
    return map(response, (response) => response.toString());
  }
}

/**
 * Map the source async iterable to a new async iterable
 * @param source
 * @param selector
 */
const map = async function* <T, T2>(
  source: AsyncIterable<T>,
  selector: (item: T, index: number) => T2 | Promise<T2>,
) {
  let i = 0;
  for await (const item of source) {
    const result = await selector(item, i++);
    yield result;
  }
};
