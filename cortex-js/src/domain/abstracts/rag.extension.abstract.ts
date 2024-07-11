import { Extension } from './extension.abstract';

export abstract class RagExtension extends Extension {
  /**
   * split documents, get embeddings, and build index
   * @param files
   * @param storePath
   */
  abstract fromFiles(
    files: string[],
    storePath: string,
    vectorDatabase?: string,
  ): Promise<void>;

  /**
   * Search the index store
   * @param query
   * @param storePath
   * @returns
   */
  abstract getResponse(
    query: string,
    llmModel: string,
    storePath: string,
    similarityTopK?: number,
  ): Promise<AsyncIterable<string>>;
}
