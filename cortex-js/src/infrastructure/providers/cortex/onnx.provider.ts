import { Injectable } from '@nestjs/common';
import CortexProvider from './cortex.provider';

@Injectable()
export default class Onnxprovider extends CortexProvider {
  productName = 'Onnx Inference Engine';
  description =
    'This extension enables chat completion API calls using the Onnx engine';
}
