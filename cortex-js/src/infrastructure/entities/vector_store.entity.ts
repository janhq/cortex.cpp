import {
  Table,
  Column,
  Model,
  PrimaryKey,
  DataType,
} from 'sequelize-typescript';

@Table({ tableName: 'vector_stores', timestamps: true })
export class VectorStoreEntity extends Model {
  @PrimaryKey
  @Column({
    type: DataType.STRING,
  })
  id: string;

  @Column({
    type: DataType.JSON,
  })
  file_ids: string[];

  @Column({
    type: DataType.STRING,
  })
  name: string;

  @Column({
    type: DataType.JSON,
  })
  chunking_strategy: object;

  @Column({
    type: DataType.STRING,
  })
  rag_extension: string;

  @Column({
    type: DataType.STRING,
  })
  vector_database: string;

  @Column({
    type: DataType.JSON,
  })
  metadata: Record<string, string>;
}
