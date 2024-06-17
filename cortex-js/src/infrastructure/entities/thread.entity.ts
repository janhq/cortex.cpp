import { Thread, ThreadToolResources } from '@/domain/models/thread.interface';
import { Entity, PrimaryColumn, Column } from 'typeorm';
import { AssistantEntity } from './assistant.entity';

@Entity('threads')
export class ThreadEntity implements Thread {
  @PrimaryColumn()
  id: string;

  @Column()
  object: 'thread';

  @Column({ name: 'title' })
  title: string;

  @Column({ type: 'simple-json' })
  assistants: AssistantEntity[];

  @Column()
  created_at: number;

  @Column({ type: 'simple-json', nullable: true })
  tool_resources: ThreadToolResources | null;

  @Column({ type: 'simple-json', nullable: true })
  metadata: any;
}
