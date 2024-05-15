import { Question, QuestionSet } from 'nest-commander';

@QuestionSet({ name: 'create-init-questions' })
export class CreateInitQuestions {
  @Question({
    type: 'list',
    message: 'Select run mode',
    name: 'runMode',
    default: 'CPU',
    choices: ['CPU', 'GPU'],
    when: () => process.platform !== 'darwin',
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
    choices: ['AVX2', 'AVX', 'AVX-512'],
    when: () => process.platform !== 'darwin',
  })
  parseContent(val: string) {
    return val;
  }
}
