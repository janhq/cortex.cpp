import { Model } from '@/domain/models/model.interface';
import { Repository } from './repository.interface';

export abstract class ModelRepository extends Repository<Model> {}
