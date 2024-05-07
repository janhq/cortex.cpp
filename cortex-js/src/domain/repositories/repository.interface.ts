export abstract class Repository<T> {
  abstract create(object: Partial<T>): Promise<T>;

  abstract findAll(): Promise<T[]>;

  abstract findOne(id: string): Promise<T | null>;

  abstract update(id: string, object: Partial<T>): Promise<void>;

  abstract remove(id: string): Promise<void>;
}
