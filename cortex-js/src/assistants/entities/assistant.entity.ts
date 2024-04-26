import {
  Assistant,
  AssistantMetadata,
  AssistantTool,
} from 'src/core/interfaces/assistant.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('assistants')
export class AssistantEntity implements Assistant {
  @PrimaryColumn()
  id: string;

  @Column()
  avatar: string;

  @Column()
  thread_location?: string;

  @Column()
  object: string;

  @Column()
  created_at: number;

  @Column()
  name: string;

  @Column()
  description?: string;

  @Column()
  model: string;

  @Column()
  instructions?: string;

  @Column({ type: 'simple-json' })
  tools?: AssistantTool[];

  @Column({ type: 'simple-array' })
  file_ids: string[];

  @Column({ type: 'simple-json' })
  metadata?: AssistantMetadata;
}
