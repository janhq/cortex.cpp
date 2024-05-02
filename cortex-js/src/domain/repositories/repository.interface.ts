export interface Repository<T> {
  create(object: Partial<T>): Promise<T>;

  findAll(): Promise<T[]>;

  findOne(id: string): Promise<T | null>;

  update(id: string, object: Partial<T>): Promise<void>;

  remove(id: string): Promise<void>;
}
