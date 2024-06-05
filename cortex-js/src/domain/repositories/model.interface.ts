import { Model } from '../models/model.interface';
import { Repository } from './repository.interface';

export abstract class ModelRepository extends Repository<Model> {}
