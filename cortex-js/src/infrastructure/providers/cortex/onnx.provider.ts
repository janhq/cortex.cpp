import { Injectable } from '@nestjs/common';
import CortexProvider from './cortex.provider';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';

@Injectable()
export default class Onnxprovider extends CortexProvider {
  name = Engines.onnx;
  productName = 'Onnx Inference Engine';
  description =
    'This extension enables chat completion API calls using the Onnx engine';
}
