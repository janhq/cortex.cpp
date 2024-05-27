import { Injectable } from '@nestjs/common';
import { AssistantsUsecases } from '../assistants/assistants.usecases';

@Injectable()
export class SeedService {
  public constructor(private readonly assistantsUsecases: AssistantsUsecases) {}

  public async seed() {
    await this.assistantsUsecases.seed();
  }
}
