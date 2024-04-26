import {
  ChatCompletionRole,
  ErrorCode,
  Message,
  MessageStatus,
  ThreadContent,
} from 'src/core/interfaces/message.interface';
import { Column, Entity, PrimaryColumn } from 'typeorm';

@Entity('messages')
export class MessageDto implements Message {
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

  @Column()
  updated: number;

  @Column({ type: 'simple-json', nullable: true })
  metadata?: Record<string, unknown>;

  @Column()
  type?: string;

  @Column()
  error_code?: ErrorCode;
}
