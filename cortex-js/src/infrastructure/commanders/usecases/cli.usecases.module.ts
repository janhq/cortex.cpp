import { Module } from "@nestjs/common";
import { InitCliUsecases } from "./init.cli.usecases";
import { HttpModule } from "@nestjs/axios";

@Module({
    imports: [HttpModule],
    controllers: [],
    providers: [InitCliUsecases],
    exports: [InitCliUsecases],
  })
  export class CliUsecasesModule {}