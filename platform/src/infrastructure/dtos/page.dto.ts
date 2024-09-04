import { ApiProperty } from '@nestjs/swagger';
import { IsArray } from 'class-validator';

export class PageDto<T> {
  @ApiProperty()
  readonly object: string;

  @IsArray()
  @ApiProperty({ isArray: true })
  readonly data: T[];

  @ApiProperty()
  readonly first_id: string | undefined;

  @ApiProperty()
  readonly last_id: string | undefined;

  @ApiProperty()
  readonly has_more: boolean;

  constructor(data: T[], hasMore: boolean, firstId?: string, lastId?: string) {
    this.object = 'list';
    this.data = data;
    this.first_id = firstId;
    this.last_id = lastId;
    this.has_more = hasMore;
  }
}
