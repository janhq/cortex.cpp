import { Module } from "@nestjs/common";
import { InitCliUsecases } from "./init.cli.usecases";
import { HttpModule } from "@nestjs/axios";
import { ModelsCliUsecases } from "./models.cli.usecases";
import { ModelsModule } from "@/usecases/models/models.module";

@Module({
    imports: [HttpModule, ModelsModule],
    controllers: [],
    providers: [InitCliUsecases, ModelsCliUsecases],
    exports: [InitCliUsecases, ModelsCliUsecases],
  })
  export class CliUsecasesModule {}