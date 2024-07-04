import { MIN_CUDA_VERSION } from "@/infrastructure/constants/cortex";
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
      const [currentMajor, currentMinor] = version.split('.').map(Number);
      const [requiredMajor, requiredMinor] = MIN_CUDA_VERSION.split('.').map(Number);
      const isMatchRequired = currentMajor > requiredMajor || (currentMajor === requiredMajor && currentMinor >= requiredMinor);
      if (!isMatchRequired) {
        console.error(`CUDA version ${version} is not compatible with TensorRT-LLM models. Required version: ${MIN_CUDA_VERSION}`);
        process.exit(1);
      }
      } catch (e) {
        console.error(e.message ?? e);
        process.exit(1);
      }
    
  }
};
