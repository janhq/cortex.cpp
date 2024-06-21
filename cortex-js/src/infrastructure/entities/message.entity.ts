import type {
  Message,
  MessageContent,
  MessageIncompleteDetails,
  MessageAttachment,
} from '@/domain/models/message.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('messages')
export class MessageEntity implements Message {
  @PrimaryColumn({ type: String })
  id: string;

  @Column({ type: String })
  object: 'thread.message';

  @Column({ type: String })
  thread_id: string;

  @Column({ type: String, nullable: true })
  assistant_id: string | null;

  @Column({ type: String })
  role: 'user' | 'assistant';

  @Column({ type: String })
  status: 'in_progress' | 'incomplete' | 'completed';

  @Column({ type: 'simple-json', nullable: true })
  metadata: any | null;

  @Column({ type: String, nullable: true })
  run_id: string | null;

  @Column({ type: Number, nullable: true })
  completed_at: number | null;

  @Column({ type: 'simple-json' })
  content: MessageContent[];

  @Column({ type: 'simple-json', nullable: true })
  incomplete_details: MessageIncompleteDetails | null;

  @Column({ type: Number })
  created_at: number;

  @Column({ type: 'simple-json' })
  attachments: MessageAttachment[];

  @Column({ type: Number, nullable: true })
  incomplete_at: number | null;
}
