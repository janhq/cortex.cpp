import { Injectable } from '@nestjs/common';
import CortexProvider from './cortex.provider';

@Injectable()
export default class TensorrtLLMProvider extends CortexProvider {
  productName = 'TensorrtLLM Inference Engine';
  description =
    'This extension enables chat completion API calls using the TensorrtLLM engine';
}
