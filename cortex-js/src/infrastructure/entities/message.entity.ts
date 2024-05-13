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

  @Column()
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

  @Column()
  type?: string;

  @Column()
  error_code?: ErrorCode;
}
