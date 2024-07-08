import { Table, Column, Model, PrimaryKey, DataType } from 'sequelize-typescript';
import type { Thread, ThreadToolResources } from '@/domain/models/thread.interface';
import { AssistantEntity } from './assistant.entity';

@Table({ tableName: 'threads', timestamps: false})
export class ThreadEntity extends Model implements Thread {
  @PrimaryKey
  @Column({
    type: DataType.STRING,
  })
  id: string;

  @Column({
    type: DataType.STRING,
    defaultValue: 'thread',
  })
  object: 'thread';

  @Column({
    type: DataType.STRING,
  })
  title: string;

  @Column({
    type: DataType.JSON,
  })
  assistants: AssistantEntity[];

  @Column({
    type: DataType.INTEGER,
  })
  created_at: number;

  @Column({
    type: DataType.JSON,
    allowNull: true,
  })
  tool_resources: ThreadToolResources | null;

  @Column({
    type: DataType.JSON,
    allowNull: true,
  })
  metadata: any | null;
}
