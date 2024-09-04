import {
  Table,
  Column,
  Model,
  PrimaryKey,
  DataType,
} from 'sequelize-typescript';
import type {
  Message,
  MessageContent,
  MessageIncompleteDetails,
  MessageAttachment,
} from '@/domain/models/message.interface';

@Table({ tableName: 'messages', timestamps: false })
export class MessageEntity extends Model implements Message {
  @PrimaryKey
  @Column({
    type: DataType.STRING,
  })
  id: string;

  @Column({
    type: DataType.STRING,
    defaultValue: 'thread.message',
  })
  object: 'thread.message';

  @Column({
    type: DataType.STRING,
  })
  thread_id: string;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  assistant_id: string | null;

  @Column({
    type: DataType.STRING,
  })
  role: 'user' | 'assistant';

  @Column({
    type: DataType.STRING,
  })
  status: 'in_progress' | 'incomplete' | 'completed';

  @Column({
    type: DataType.JSON,
    allowNull: true,
  })
  metadata: any | null;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  run_id: string | null;

  @Column({
    type: DataType.INTEGER,
    allowNull: true,
  })
  completed_at: number | null;

  @Column({
    type: DataType.JSON,
  })
  content: MessageContent[];

  @Column({
    type: DataType.JSON,
    allowNull: true,
  })
  incomplete_details: MessageIncompleteDetails | null;

  @Column({
    type: DataType.INTEGER,
  })
  created_at: number;

  @Column({
    type: DataType.JSON,
  })
  attachments: MessageAttachment[];

  @Column({
    type: DataType.INTEGER,
    allowNull: true,
  })
  incomplete_at: number | null;
}
