import { Module } from '@nestjs/common';
import CortexProvider from './cortex.provider';
import { HttpModule } from '@nestjs/axios';
import { FileManagerModule } from '@/infrastructure/services/file-manager/file-manager.module';
import Onnxprovider from './onnx.provider';
import LlamaCPPProvider from './llamacpp.provider';
import TensorrtLLMProvider from './tensorrtllm.provider';

@Module({
  imports: [HttpModule, FileManagerModule],
  providers: [
    {
      provide: 'CORTEX_PROVIDER',
      useClass: CortexProvider,
    },
    Onnxprovider,
    LlamaCPPProvider,
    TensorrtLLMProvider,
  ],
  exports: [
    {
      provide: 'CORTEX_PROVIDER',
      useClass: CortexProvider,
    },
    Onnxprovider,
    LlamaCPPProvider,
    TensorrtLLMProvider,
  ],
})
export class CortexProviderModule {}
