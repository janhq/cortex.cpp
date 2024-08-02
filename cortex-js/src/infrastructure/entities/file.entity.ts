import {
  Table,
  Column,
  Model,
  PrimaryKey,
  DataType,
} from 'sequelize-typescript';

@Table({ tableName: 'files', timestamps: false })
export class FileEntity extends Model {
  @PrimaryKey
  @Column({
    type: DataType.STRING,
  })
  id: string;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  filename: string;

  @Column({
    type: DataType.STRING,
  })
  object: string;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  purpose: string;

  @Column({
    type: DataType.NUMBER,
    allowNull: true,
  })
  bytes: number;

  @Column({
    type: DataType.DATE,
  })
  created_at: number;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  status: string;

  @Column({
    type: DataType.STRING,
    allowNull: true,
  })
  sha: string;
}
