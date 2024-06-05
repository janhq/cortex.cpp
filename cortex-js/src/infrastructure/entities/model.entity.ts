import { Model, ModelArtifact } from '@/domain/models/model.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('models')
export class ModelEntity implements Model {
  // Cortex Meta
  @PrimaryColumn()
  model: string;

  @Column()
  name: string;

  @Column()
  version: string;

  @Column({ type: 'simple-json' })
  files: string[] | ModelArtifact;

  // Model Input / Output Syntax
  @Column()
  prompt_template: string;

  @Column({ type: 'simple-json' })
  stop: string[];

  @Column()
  max_tokens: number;

  // Results Preferences
  @Column()
  top_p: number;

  @Column()
  temperature: number;

  @Column()
  frequency_penalty: number;

  @Column()
  presence_penalty: number;

  @Column()
  stream: boolean;

  // Engine Settings
  @Column()
  ctx_len: number;

  @Column()
  ngl: number;

  @Column()
  n_parallel: number;

  @Column()
  cpu_threads: number;

  @Column()
  engine: string;
}
