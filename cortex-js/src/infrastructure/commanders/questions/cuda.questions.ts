import { Question, QuestionSet } from 'nest-commander';
import { platform } from 'node:process';

@QuestionSet({ name: 'init-cuda-questions' })
export class InitCudaQuestions {
  @Question({
    type: 'list',
    message: 'Do you want to install additional dependencies for CUDA Toolkit?',
    name: 'installCuda',
    default: 'Yes',
    choices: ['Yes', 'No, I want to use my own CUDA Toolkit'],
    when: () => platform !== 'darwin',
  })
  parseRunMode(val: string) {
    return val;
  }
}
