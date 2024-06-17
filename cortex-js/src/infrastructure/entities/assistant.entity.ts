import {
  Assistant,
  AssistantResponseFormatOption,
  AssistantToolResources,
} from '@/domain/models/assistant.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('assistants')
export class AssistantEntity implements Assistant {
  @PrimaryColumn({ type: String })
  id: string;

  @Column({ type: String, nullable: true })
  avatar?: string;

  @Column({ type: String })
  object: 'assistant';

  @Column({ type: Number })
  created_at: number;

  @Column({ type: String, nullable: true })
  name: string | null;

  @Column({ type: String, nullable: true })
  description: string | null;

  @Column({ type: String })
  model: string;

  @Column({ type: String, nullable: true })
  instructions: string | null;

  @Column({ type: 'simple-json' })
  tools: any;

  @Column({ type: 'simple-json', nullable: true })
  metadata: any | null;

  @Column({ type: Number, nullable: true })
  top_p: number | null;

  @Column({ type: Number, nullable: true })
  temperature: number | null;

  @Column({ type: 'simple-json', nullable: true })
  response_format: AssistantResponseFormatOption | null;

  @Column({ type: 'simple-json', nullable: true })
  tool_resources: AssistantToolResources | null;
}
