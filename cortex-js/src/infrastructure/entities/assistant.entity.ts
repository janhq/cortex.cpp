import {
  Assistant,
  AssistantResponseFormatOption,
  AssistantToolResources,
} from '@/domain/models/assistant.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('assistants')
export class AssistantEntity implements Assistant {
  @PrimaryColumn()
  id: string;

  @Column({ nullable: true })
  avatar?: string;

  @Column()
  object: 'assistant';

  @Column()
  created_at: number;

  @Column({ type: 'simple-json', nullable: true })
  name: string | null;

  @Column({ type: 'simple-json', nullable: true })
  description: string | null;

  @Column()
  model: string;

  @Column({ type: 'simple-json', nullable: true })
  instructions: string | null;

  @Column({ type: 'simple-json' })
  tools: any;

  @Column({ type: 'simple-json', nullable: true })
  metadata: any;

  @Column({ type: 'simple-json', nullable: true })
  top_p?: number | null;

  @Column({ type: 'simple-json', nullable: true })
  temperature?: number | null;

  @Column({ type: 'simple-json', nullable: true })
  response_format?: AssistantResponseFormatOption;

  @Column({ type: 'simple-json', nullable: true })
  tool_resources?: AssistantToolResources;
}
