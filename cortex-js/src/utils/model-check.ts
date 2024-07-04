import { getCudaVersion } from "./cuda";

export const checkModelCompatibility = async (modelId: string) => {  
  if (modelId.includes('onnx') && process.platform !== 'win32') {
    console.error('The ONNX engine does not support this OS yet.');
    process.exit(1);
  }

  if (modelId.includes('tensorrt-llm') ) {
    if(process.platform === 'darwin'){
      console.error('Tensorrt-LLM models are not supported on this OS');
      process.exit(1);
    }

    try{
      const version = await getCudaVersion();
      console.log('Checking model compatibility...', version);
      } catch (e) {
        console.error(e.message ?? e);
        process.exit(1);
      }
    
  }
};
