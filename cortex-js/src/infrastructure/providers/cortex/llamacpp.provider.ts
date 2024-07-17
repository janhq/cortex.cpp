import { Injectable } from '@nestjs/common';
import CortexProvider from './cortex.provider';

@Injectable()
export default class LlamaCPPProvider extends CortexProvider {
  productName = 'LlamaCPP Inference Engine';
  description =
    'This extension enables chat completion API calls using the LlamaCPP engine';
}
