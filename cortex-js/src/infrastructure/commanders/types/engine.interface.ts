export enum Engines {
  llamaCPP = 'cortex.llamacpp',
  onnx = 'cortex.onnx',
  tensorrtLLM = 'cortex.tensorrt-llm',

  // Remote engines
  groq = 'groq',
  mistral = 'mistral',
  openai = 'openai',
  anthropic = 'anthropic',
}

export const EngineNamesMap = {
  [Engines.llamaCPP]: 'llamacpp',
  [Engines.onnx]: 'onnx',
  [Engines.tensorrtLLM]: 'tensorrt-llm',
};
