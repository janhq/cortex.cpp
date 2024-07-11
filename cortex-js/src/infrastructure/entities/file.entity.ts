import {
  Table,
  Column,
  Model,
  PrimaryKey,
  DataType,
  Unique,
} from 'sequelize-typescript';

@Table({ tableName: 'vector_stores', timestamps: false })
export class FileEntity extends Model {
  @PrimaryKey
  @Column({
    type: DataType.STRING,
  })
  id: string;

  @Unique
  @Column({
    type: DataType.STRING,
  })
  name: string;

  @Column({
    type: DataType.STRING,
  })
  status: string;

  @Column({
    type: DataType.STRING,
  })
  sha: string;
}
