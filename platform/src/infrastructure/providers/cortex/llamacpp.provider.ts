import { Injectable } from '@nestjs/common';
import CortexProvider from './cortex.provider';
import { Engines } from '@/infrastructure/commanders/types/engine.interface';

@Injectable()
export default class LlamaCPPProvider extends CortexProvider {
  name = Engines.llamaCPP;
  productName = 'LlamaCPP Inference Engine';
  description =
    'This extension enables chat completion API calls using the LlamaCPP engine';
}
