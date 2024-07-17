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
  [Engines.tensorrtLLM]: 'tensorrtLLM',
  [Engines.groq]: 'groq',
  [Engines.mistral]: 'mistral',
  [Engines.openai]: 'openai',
  [Engines.anthropic]: 'anthropic',
};
