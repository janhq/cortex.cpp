import {
  ChatCompletionRole,
  ErrorCode,
  Message,
  MessageMetadata,
  MessageStatus,
  ThreadContent,
} from '@/domain/models/message.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('messages')
export class MessageEntity implements Message {
  @PrimaryColumn()
  id: string;

  @Column()
  object: string;

  @Column()
  thread_id: string;

  @Column({ nullable: true })
  assistant_id?: string;

  @Column()
  role: ChatCompletionRole;

  @Column({ type: 'simple-json' })
  content: ThreadContent[];

  @Column()
  status: MessageStatus;

  @Column()
  created: number;

  @Column({ nullable: true })
  updated?: number;

  @Column({ type: 'simple-json', nullable: true })
  metadata?: MessageMetadata;

  @Column({ nullable: true })
  type?: string;

  @Column({ nullable: true })
  error_code?: ErrorCode;

  @Column({ type: 'simple-json', nullable: true })
  attachments?: any[];
}
