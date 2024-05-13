import {
  Model,
  ModelArtifact,
  ModelFormat,
  ModelMetadata,
  ModelRuntimeParams,
  ModelSettingParams,
} from '@/domain/models/model.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('models')
export class ModelEntity implements Model {
  @PrimaryColumn()
  id: string;

  @Column()
  object: string;

  @Column()
  version: string;

  @Column()
  format: ModelFormat;

  @Column({ type: 'simple-json' })
  sources: ModelArtifact[];

  @Column()
  name: string;

  @Column()
  created: number;

  @Column()
  description: string;

  @Column({ type: 'simple-json' })
  settings: ModelSettingParams;

  @Column({ type: 'simple-json' })
  parameters: ModelRuntimeParams;

  @Column({ type: 'simple-json' })
  metadata: ModelMetadata;

  @Column()
  engine: string;
}
