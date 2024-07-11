export abstract class VectorDatabase {
  abstract onLoad(): void;

  abstract createCollection(collectionName: string, vectors: any): void;

  abstract search(collectionName: string, query: string): Promise<any>;

  abstract upsert(collectionName: string, points: any[]): Promise<void>;
}
