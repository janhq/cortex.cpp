import { Model } from '@/domain/models/model.interface';
import { Repository } from './repository.interface';

export abstract class ModelRepository extends Repository<Model> {
  abstract loadModelByFile(
    modelId: string,
    filePath: string,
    modelFile: string,
  ): Promise<Model | null>;
}
