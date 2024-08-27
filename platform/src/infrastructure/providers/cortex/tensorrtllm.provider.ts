import { Injectable } from '@nestjs/common';
import CortexProvider from './cortex.provider';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';

@Injectable()
export default class TensorrtLLMProvider extends CortexProvider {
  name = Engines.tensorrtLLM;
  productName = 'TensorrtLLM Inference Engine';
  description =
    'This extension enables chat completion API calls using the TensorrtLLM engine';
}
