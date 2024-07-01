export const checkModelCompatibility = (modelId: string) => {
  if (modelId.includes('onnx') && process.platform !== 'win32') {
    console.error('The ONNX engine does not support this OS yet.');
    process.exit(1);
  }

  if (modelId.includes('tensorrt-llm') && process.platform === 'darwin') {
    console.error('Tensorrt-LLM models are not supported on this OS');
    process.exit(1);
  }
};
