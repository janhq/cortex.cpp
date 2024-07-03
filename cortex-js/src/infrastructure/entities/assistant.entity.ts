import { Table, Column, Model, PrimaryKey, DataType } from 'sequelize-typescript';
import { Assistant } from '@/domain/models/assistant.interface';
import type {
  AssistantToolResources,
  AssistantResponseFormatOption,
} from '@/domain/models/assistant.interface';

@Table({ tableName: 'assistants' })
export class AssistantEntity extends Model implements Assistant {
  @PrimaryKey
  @Column({
    type: DataType.STRING,
  })
  id: string;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  avatar?: string;

  @Column({
    type: DataType.STRING,
    defaultValue: 'assistant',
  })
  object: 'assistant';

  @Column({
    type: DataType.INTEGER,
  })
  created_at: number;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  name: string | null;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  description: string | null;

  @Column({
    type: DataType.STRING,
  })
  model: string;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  instructions: string | null;

  @Column({
    type: DataType.JSON,
  })
  tools: any;

  @Column({
    type: DataType.JSON,
    allowNull: true,
  })
  metadata: any | null;

  @Column({
    type: DataType.FLOAT,
    allowNull: true,
  })
  top_p: number | null;

  @Column({
    type: DataType.FLOAT,
    allowNull: true,
  })
  temperature: number | null;

  @Column({
    type: DataType.JSON,
    allowNull: true,
  })
  response_format: AssistantResponseFormatOption | null;

  @Column({
    type: DataType.JSON,
    allowNull: true,
  })
  tool_resources: AssistantToolResources | null;
}
