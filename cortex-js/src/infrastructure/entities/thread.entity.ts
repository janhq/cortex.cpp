import type { Thread, ThreadToolResources } from '@/domain/models/thread.interface';
import { Entity, PrimaryColumn, Column } from 'typeorm';
import { AssistantEntity } from './assistant.entity';

@Entity('threads')
export class ThreadEntity implements Thread {
  @PrimaryColumn({ type: String })
  id: string;

  @Column({ type: String })
  object: 'thread';

  @Column({ type: String, name: 'title' })
  title: string;

  @Column({ type: 'simple-json' })
  assistants: AssistantEntity[];

  @Column({ type: Number })
  created_at: number;

  @Column({ type: 'simple-json', nullable: true })
  tool_resources: ThreadToolResources | null;

  @Column({ type: 'simple-json', nullable: true })
  metadata: any | null;
}
