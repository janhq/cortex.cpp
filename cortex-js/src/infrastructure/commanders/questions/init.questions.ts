import { Question, QuestionSet } from 'nest-commander';
import { platform } from 'node:process';

@QuestionSet({ name: 'init-run-mode-questions' })
export class InitRunModeQuestions {
  @Question({
    type: 'list',
    message: 'Select run mode',
    name: 'runMode',
    default: 'CPU',
    choices: ['CPU', 'GPU'],
    when: () => platform !== 'darwin',
  })
  parseRunMode(val: string) {
    return val;
  }

  @Question({
    type: 'list',
    message: 'Select GPU type',
    name: 'gpuType',
    default: 'Nvidia',
    choices: ['Nvidia', 'Others (Vulkan)'],
    when: (answers: any) => answers.runMode === 'GPU',
  })
  parseGPUType(val: string) {
    return val;
  }

  @Question({
    type: 'list',
    message: 'Select CPU instructions set',
    name: 'instructions',
    choices: ['AVX2', 'AVX', 'AVX512'],
    when: () => platform !== 'darwin',
  })
  parseContent(val: string) {
    return val;
  }
}
