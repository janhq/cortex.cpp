import { Table, Model, PrimaryKey, Column, DataType } from "sequelize-typescript";

@Table({ tableName: 'file_batches', timestamps: false })
export class FileBatchEntity extends Model {
  @PrimaryKey
  @Column({
    type: DataType.STRING
  })
  id: string;

  @Column({
    type: DataType.STRING,
  })
  object: string;

  @Column({
    type: DataType.STRING,
  })
  vector_store_id: string;

  @Column({
    type: DataType.STRING,
  })
  status: string;

  @Column({
    type: DataType.JSON,
  })
  file_counts: Object
}

